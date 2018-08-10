#ifndef AUDIO_CPP
#define AUDIO_CPP


#include <stdio.h>
#include <math.h>
#include <iostream>
#include <cstring>
#include <time.h>  
#include <vector>
#include <algorithm>

#include "audio.h"
#include "input.h"
#include "src/util.cpp"


class ScopedPaHandler
{
public:
	ScopedPaHandler()
		: _result(Pa_Initialize())
	{
	}
	~ScopedPaHandler()
	{
		if (_result == paNoError)
		{
			Pa_Terminate();
		}
	}

	PaError result() const { return _result; }

private:
	PaError _result;
};

Audio::Audio(): stream(0), left_phase(0), right_phase(0){
	//dont do anything here ( unless you check for duplicate threads later)
	// createWaveTable();
	// //start the audio thread (basically just sleeps while audio runs in the background, not sure how to do this correctly)
	// thread = std::thread(&Audio::run, this );
	
}

Audio::Audio(Input* in){
	inputs = in;
	createWaveTable();

	//start the audio thread (basically just sleeps while audio runs in the background, not sure how to do this correctly)
	thread = std::thread(&Audio::run, this );
}

Audio::~Audio() {
	stop();
	close();
	thread.join();
}

void Audio::createWaveTable(){
	/* initialise sinusoidal wavetable */ //replace this with something more interesting
	for ( int i = 0; i < TABLE_SIZE; i++ )
	{
		sine[i] = (float) sin(  (double)i / (double)TABLE_SIZE  * M_PI * 2. )  ;
	}
}

void Audio::loadByteWaveTable(unsigned char* table, int size){
	loadedWaveTable = table;
	loadedWaveTableSize = size;
}


bool Audio::run(){
	//set up last frame buffer last write location
	lastFrameL = std::begin(frameBuffer[0]);
	lastFrameR = std::begin(frameBuffer[1]);

	ScopedPaHandler paInit;
	if ( paInit.result() != paNoError ) {
		fprintf( stderr, "An error occured while using the portaudio stream\n" );
		fprintf( stderr, "Error number: %d\n", paInit.result() );
		fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( paInit.result() ) );
		return false;
	}

	if (open(Pa_GetDefaultOutputDevice()))
	{
		printf("Playing a sin wave\n" );

			if (start()) {
				
				while(1){};
			}
	}
	return true;
}



bool Audio::open(PaDeviceIndex index)
{
	PaStreamParameters outputParameters;

	outputParameters.device = index;
	if (outputParameters.device == paNoDevice) {
		return false;
	}

	const PaDeviceInfo* pInfo = Pa_GetDeviceInfo(index);
	if (pInfo != 0)
	{
		printf("Output device name: '%s'\r", pInfo->name);
	}

	outputParameters.channelCount = 2;       /* stereo output */
	outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
	outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;

	PaError err = Pa_OpenStream(
	                  &stream,
	                  NULL, /* no input */
	                  &outputParameters,
	                  SAMPLE_RATE,
	                  //paFramesPerBufferUnspecified,
	                  FRAMES_PER_BUFFER,
	                  0,      /* we won't output out of range samples so don't bother clipping them */
	                  &Audio::paCallback,
	                  this            /* Using 'this' for userData so we can cast to Audio* in paCallback method */
	              );

	if (err != paNoError)
	{
		/* Failed to open stream to device !!! */
		return false;
	}

	err = Pa_SetStreamFinishedCallback( stream, &Audio::paStreamFinished );

	if (err != paNoError)
	{
		Pa_CloseStream( stream );
		stream = 0;

		return false;
	}

	return true;
}

bool Audio::close()
{
	if (stream == 0)
		return false;

	PaError err = Pa_CloseStream( stream );
	stream = 0;

	return (err == paNoError);
}


bool Audio::start()
{
	if (stream == 0)
		return false;

	PaError err = Pa_StartStream( stream );
	begin = clock();

	return (err == paNoError);
}

bool Audio::stop()
{
	if (stream == 0)
		return false;

	PaError err = Pa_StopStream( stream );

	return (err == paNoError);
}

//port audio specific handlers
int Audio::paCallbackMethod(const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags)
{
	float *out = (float*)outputBuffer;

	//int thisTableSize = TABLE_SIZE;
	int thisTableSize = TABLE_SIZE;

	unsigned long i;

	(void) timeInfo; /* Prevent unused variable warnings. */
	(void) statusFlags;
	(void) inputBuffer;

	createWaveTable();

	float hertz = 32.70; // C1

	float carrierOctaveRange = 8.;
	float modOctaveRange = 8.;

	float playSpeed = (thisTableSize/(float)SAMPLE_RATE * hertz) * (pow(2, (int)(inputs->getCV(0) * (carrierOctaveRange * 12))/12. ) ); //(quantized)don't cast if you want free frequency control

	float modSpeed = playSpeed * (pow(2, (int)(inputs->getCV(1) * (modOctaveRange * 12))/12. ) );
	//std::cout<< thisTableSize <<std::endl;

	// float bufferCopyL[FRAMES_PER_BUFFER];
	// float bufferCopyR[FRAMES_PER_BUFFER];
	if(thisTableSize != 0){
		for ( i = 0; i < framesPerBuffer; i++ )
		{

			mod_phase += modSpeed;
			while ( mod_phase >= thisTableSize ) mod_phase -= thisTableSize;

			carrier_phase += playSpeed;
			if(carrier_phase < 0) carrier_phase = 0;
			while ( carrier_phase >= thisTableSize ) carrier_phase -= thisTableSize;

			right_phase += playSpeed + sine[(int)mod_phase] ;
			if(right_phase < 0) right_phase = 0;
			while ( right_phase >= thisTableSize ) right_phase -= thisTableSize;

			bufferCopyL[i] = sine[(int)carrier_phase];
			bufferCopyR[i] = sine[(int)mod_phase];

			*out++ = sine[(int)right_phase];  /* left */
			//buffer[0][i]= *out;
			float left = *out;
			*out++ = sine[(int)right_phase];  /* right */
			float right = *out;
		}
	}
	else{
		std::cout<< "WARNING: No Wavetable Loaded."<<std::endl;
	}
	
	recordMutex.lock();
	if( lastFrameL == std::end(frameBuffer[0]) ) {
		lastFrameL = std::begin(frameBuffer[0]);
		newBufferFlag = true;
	}
	if( lastFrameR == std::end(frameBuffer[1]) ) {
		lastFrameR = std::begin(frameBuffer[1]);
		newBufferFlag = true;
	}

	lastFrameL = std::copy(std::begin(bufferCopyL), std::end(bufferCopyL), lastFrameL);
	lastFrameR = std::copy(std::begin(bufferCopyR), std::end(bufferCopyR), lastFrameR);
	recordMutex.unlock();



	return paContinue;
}


void Audio::paStreamFinishedMethod()
{
	printf( "Stream Completed: %s\n", message );
}

bool Audio::hasNewBuffer(){
	return newBufferFlag;
}

std::vector<float> Audio::getBuffer(int index){
	newBufferFlag = false;
	recordMutex.lock();
	std::vector<float> out = frameBuffer[index];
	recordMutex.unlock();
	return out;
}

int Audio::getFramesPerBuffer(){
	return FRAMES_PER_BUFFER;
}


#endif