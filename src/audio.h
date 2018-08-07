#ifndef AUDIO_H
#define AUDIO_H

#include "lib/portaudio.h"
#include <thread>
#include <mutex>
#include <list>
#include <vector>
#include <time.h>

#define SAMPLE_RATE   (192000)
#define FRAMES_PER_BUFFER   (2560)
#define TABLE_SIZE   (800)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

#define RECORD_BUFFER  (2560)

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
	bool hasNewBuffer();
	std::vector<float> getBuffer(int index);

	void loadByteWaveTable(unsigned char* table, int size);

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
	unsigned char* loadedWaveTable;
	int loadedWaveTableSize = 0;
	bool newBufferFlag = false;
	std::vector< float > frameBuffer[2] = {std::vector<float>(RECORD_BUFFER, 0.0), std::vector<float>(RECORD_BUFFER, 0.0)};
	std::vector<float>::iterator lastFrameL;
	std::vector<float>::iterator lastFrameR;
	//std::vector<float> buffer = std::vector<float>(RECORD_BUFFER, 0.0);

	
	float left_phase;
	float carrier_phase;
	float mod_phase;
	float right_phase;
	float fm_phase = 0.;
	char message[20];
	std::thread thread;
	std::mutex recordMutex;
	

	std::vector<float> bufferCopyL = std::vector<float>(FRAMES_PER_BUFFER, 0.);
	std::vector<float> bufferCopyR = std::vector<float>(FRAMES_PER_BUFFER, 0.);

	clock_t begin;
};

#endif