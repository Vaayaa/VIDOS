#ifndef AUDIO_H
#define AUDIO_H

#include "lib/portaudio.h"
#include <thread>

#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER   (1024)
#define TABLE_SIZE   (200)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

class Audio
{
public:
	Audio();
	~Audio();

	bool open(PaDeviceIndex index);
	bool close();
	bool start();
	bool stop();

	bool run();

	int getFramesPerBuffer();
	float* getBuffer();

private:

	int paCallbackMethod(const void *inputBuffer, void *outputBuffer,
	                     unsigned long framesPerBuffer,
	                     const PaStreamCallbackTimeInfo* timeInfo,
	                     PaStreamCallbackFlags statusFlags);

	/* This routine will be called by the PortAudio engine when audio is needed.
	** It may called at interrupt level on some machines so don't do anything
	** that could mess up the system like calling malloc() or free().
	*/
	static int paCallback( const void *inputBuffer, void *outputBuffer,
	                              unsigned long framesPerBuffer,
	                              const PaStreamCallbackTimeInfo* timeInfo,
	                              PaStreamCallbackFlags statusFlags,
	                              void *userData )
	{
		/* Here we cast userData to Sine* type so we can call the instance method paCallbackMethod, we can do that since
		   we called Pa_OpenStream with 'this' for userData */
		return ((Audio*)userData)->paCallbackMethod(inputBuffer, outputBuffer,
		        framesPerBuffer,
		        timeInfo,
		        statusFlags);
	}

	void paStreamFinishedMethod();
	/*
	* This routine is called by portaudio when playback is done.
	*/
	static void paStreamFinished(void* userData)
	{
		return ((Audio*)userData)->paStreamFinishedMethod();
	}

	PaStream *stream;
	float sine[TABLE_SIZE];
	float buffer[FRAMES_PER_BUFFER][2] = {0};
	
	int left_phase;
	int right_phase;
	char message[20];
	std::thread thread;

	float bufferCopy[FRAMES_PER_BUFFER + FRAMES_PER_BUFFER] = {0};

};

#endif