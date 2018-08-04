#ifndef AUDIO_H
#define AUDIO_H

#include "lib/portaudio.h"
#include <thread>
#include <mutex>
#include <list>
#include <time.h>

#define SAMPLE_RATE   (88200)
#define FRAMES_PER_BUFFER   (1024)
#define TABLE_SIZE   (800)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

#define RECORD_BUFFER  (4096)

class Input;

class Audio
{
public:
	Audio();
	Audio(Input* in);
	~Audio();

	bool open(PaDeviceIndex index);
	bool close();
	bool start();
	bool stop();

	bool run();

	int getFramesPerBuffer();
	std::list<float> getBuffer(int index);

private:
	void createWaveTable();

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

	Input* inputs;

	PaStream *stream;
	float sine[TABLE_SIZE];
	std::list<float> buffer[2] = {std::list<float>(RECORD_BUFFER, 0.0), std::list<float>(RECORD_BUFFER, 0.0)};
	
	float left_phase;
	float right_phase;
	float modPhase = 0.;
	char message[20];
	std::thread thread;
	std::mutex recordMutex;

	float bufferCopy[FRAMES_PER_BUFFER + FRAMES_PER_BUFFER] = {0};

	clock_t begin;
};

#endif