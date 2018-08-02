#ifndef AUDIO_CPP
#define AUDIO_CPP

#include <stdio.h>
#include <math.h>
#include <audio.h>
#include <iostream>
#include <cstring>


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
	/* initialise sinusoidal wavetable */ //replace this with something more interesting
	for ( int i = 0; i < TABLE_SIZE; i++ )
	{
		sine[i] = (float) sin( ((double)i / (double)TABLE_SIZE) * M_PI * 2. );
	}

	//start the audio thread (basically just sleeps while audio runs in the background, not sure how to do this correctly)
	thread = std::thread(&Audio::run, this );
	
}

Audio::~Audio() {
	stop();
	close();
	thread.join();
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
		printf("Playing a sin wave" );

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
	                  paClipOff,      /* we won't output out of range samples so don't bother clipping them */
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

	for ( i = 0; i < framesPerBuffer; i++ )
	{
		left_phase += 3;
		if ( left_phase >= TABLE_SIZE ) left_phase -= TABLE_SIZE;
		right_phase += 1; /* higher pitch so we can distinguish left and right. */
		if ( right_phase >= TABLE_SIZE ) right_phase -= TABLE_SIZE;

		*out++ = sine[left_phase];  /* left */
		*out++ = sine[right_phase];  /* right */
	}
	
	memcpy(bufferCopy, out, framesPerBuffer * 2.);


	return paContinue;
}


void Audio::paStreamFinishedMethod()
{
	printf( "Stream Completed: %s\n", message );
}

float* Audio::getBuffer(){
	return bufferCopy;
}

int Audio::getFramesPerBuffer(){
	return FRAMES_PER_BUFFER;
}


#endif