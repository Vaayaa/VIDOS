#ifndef AUDIO_CPP
#define AUDIO_CPP


#include <stdio.h>
#include <math.h>
#include <iostream>
#include <cstring>

#include "audio.h"
#include "input.h"


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

Audio::Audio(): stream(0), left_phase(0), right_phase(0) {

	createWaveTable();
	//start the audio thread (basically just sleeps while audio runs in the background, not sure how to do this correctly)
	thread = std::thread(&Audio::run, this );
	
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
		sine[i] = tanh( (float) sin(  log((double)i / (double)TABLE_SIZE )   * M_PI * 2. * inputs->getCV(1)) * sin( ( (double)i / (double)TABLE_SIZE ) * ( log(inputs->getCV(2) * 10.)  * M_PI * 2. * inputs->getCV(1) ) )   );
	}
}


bool Audio::run(){
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

	unsigned long i;

	(void) timeInfo; /* Prevent unused variable warnings. */
	(void) statusFlags;
	(void) inputBuffer;

	createWaveTable();

	for ( i = 0; i < framesPerBuffer; i++ )
	{
		//std::cout<< inputs->getCV(0) <<std::endl;
		float playSpeed = 5 * inputs->getCV(0);
		left_phase += playSpeed;
		if ( left_phase >= TABLE_SIZE ) left_phase -= TABLE_SIZE;
		right_phase += playSpeed; /* higher pitch so we can distinguish left and right. */
		if ( right_phase >= TABLE_SIZE ) right_phase -= TABLE_SIZE;


		*out++ = sine[(int)floor(left_phase)];  /* left */
		//buffer[0][i]= *out;
		float left = *out;
		*out++ = sine[(int)floor(right_phase)];  /* right */
		float right = *out;
		recordMutex.lock();
		buffer[0].push_back(left);
		buffer[0].pop_front();
		buffer[1].push_back(right);
		buffer[1].pop_front();
		recordMutex.unlock();
	}
	
	//memcpy(bufferCopy, out, framesPerBuffer * 2.);


	return paContinue;
}


void Audio::paStreamFinishedMethod()
{
	printf( "Stream Completed: %s\n", message );
}

std::list<float> Audio::getBuffer(int index){
	recordMutex.lock();
	std::list<float> out = std::list<float>(buffer[index]);
	recordMutex.unlock();
	return out;
}

int Audio::getFramesPerBuffer(){
	return FRAMES_PER_BUFFER;
}


#endif