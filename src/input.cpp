#ifndef INPUT_CPP
#define INPUT_CPP

#include "input.h"
#include <iostream>
#include <unistd.h>

#include <string.h>
#include <fstream>
#include <fcntl.h>

//wiringPi includes
#define BASE 100
#define SPI_CHAN 0
#include <wiringPi.h>
#include "mcp3004.h"


Input::Input() {
	onButton = 0;
	if (useSerial) {
		std::cout << "Using Serial (Arduino) for Input." << std::endl;
		setupSerial();
	}
	else {
		std::cout << "Using MCP3008 for Input." << std::endl;
		setupADC();
	}
}

Input::~Input() {
	if (threadRunning) {
		threadRunning = false;
		inputThread.join();
	}
}

void Input::update() {

}

void Input::addButtonCallback(std::function<void(bool)> buttonCallback) {
	onButton = buttonCallback;
}

bool Input::setupADC() {
	//initialize wiringPi
	if (wiringPiSetup() == -1)
	{
		std::cout << "Input Error: WiringPi setup failure" << std::endl;
		return false;
	}
	mcp3004Setup(BASE, SPI_CHAN);
	//start input thread
	threadRunning = true;
	inputThread = std::thread(&Input::readADC, this);
	return true;
}

bool Input::readADC() {
	while (threadRunning) {
		//read mcp3008
		for ( int chan = 0; chan < 8 ; chan++)
		{
			//read in ADC values
			int val = analogRead( BASE + chan);
			int smoothVal = smooth(val, lastSmoothCV[chan]);
			if(smoothVal > -1){
				lastSmoothCV[chan] = val;
				setCV(chan, val / 1024.0);

				
			}
			// if( chan == 4)
			// 	printf("%d: %d lst: %d \n", chan, val, lastSmoothCV[chan]);
		}

		readSwitches();

		
	}



	return true;
}

bool Input::readSwitches(){
	//read switches

	set3PosSwitch(0, 4, 5);
	set3PosSwitch(1, 0, 2);
	set3PosSwitch(2, 22, 23);


	// // bool pressedDown = false;
	// inputMutex.lock();
	// if (val != buttonIn) {
	// 	if (!buttonIn) {
	// 		pressedDown = true;
	// 	}
	// 	buttonIn = val;
	// }
	// inputMutex.unlock();
	// if (onButton && pressedDown) {
	// 	onButton(buttonIn);
	// }
	//printf("Button%d\n", buttonIn);
	return true;
}



//Serial input - from arduino

//reads serial inputs from Arduino, make sure to setupSerial() before calling this function
bool Input::readSerial() {

	while (threadRunning) {
		char buff[0x1000];
		ssize_t rd = read(serialFd, buff, 100);
		if (rd != 0) {
			if (strchr(buff, '\n') != NULL) {
				char* tok;

				int index = -1;
				tok = strtok(buff, " ");
				if (tok != NULL) {

					index = atoi(tok);
					//std::cout<< "Index: "<< index <<std::endl;
				}
				else {
					//~ return false;
				}
				tok = strtok(NULL, "\n");

				if (tok != NULL) {
					if(index == 10){
					 //printf("Value: %s\n", tok);
					}
					int val = atoi(tok);
					if (val < 0) {
						val = 0;
					}
					else if (val > 1024) {
						val = 1024;
					}

					if (index >= 10 && index < 30) { //cv input

						setCV(index - 10, val/1024.0);
					}


					else if ( index >= 30 && index < 40) { //button Input
						inputMutex.lock();

						buttonIn = val;
						
						inputMutex.unlock();

						onButton(buttonIn);

					}

				}
			}


		}
	}

	return true;
}

bool Input::setupSerial() {
	const char *dev = "/dev/ttyUSB0";

	serialFd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
	fcntl(serialFd, F_SETFL, 0);
	if (serialFd == -1) {
		fprintf(stderr, "Cannot open %s: %s.\n", dev, strerror(errno));
		return false;
	}

	threadRunning = true;
	inputThread = std::thread(&Input::readSerial, this);

	return true;
}

void Input::setCV(int index, float val) {
	// if(index == 4)
	// std::cout << "Setting CV " << index << ": "<< val<< std::endl;

	//~ smoothVal = smooth(val, lastCV[chan-3]);
	//~ if(smoothVal != -1){
	//~ inputMutex.lock();
	//~ cvIn[chan-3] = val/1024.0;
	//~ inputMutex.unlock();
	//lastCV[chan-3] = val;
	//~ }

	inputMutex.lock();
	clock_t readtime = clock();
	cvIn[index] = val;
	lastCV[index].push_back(val);
	lastCV[index].erase(lastCV[index].begin());
	//save time when the read was performed
	lastCVRead[index] = (float) readtime / (CLOCKS_PER_SEC);
	inputMutex.unlock();
}

void Input::set3PosSwitch(int index, int pin1, int pin2){

	int r1 = digitalRead(pin1);
	int r2 = digitalRead(pin2);

	// 0 is off (center) 1 is up 2 is down
	int switchPos = 0;
	if(r1){
		switchPos = 1;
	}
	if (r2){
		switchPos = 2;
	}

	inputMutex.lock();
	switches[index] = switchPos;
	inputMutex.unlock();

	// if(index == 1)
	// std::cout << "Setting Switch " << index << " " << r1 <<"&" << r2 << ": "<< switches[index]<< std::endl;

}

float Input::getCV(int i) {
	float ret;
	Input::inputMutex.lock();
	ret = cvIn[i];
	Input::inputMutex.unlock();
	return ret;
}

int Input::getSwitch(int i){
	float ret;
	Input::inputMutex.lock();
	ret = switches[i];
	Input::inputMutex.unlock();
	return ret;
}

std::vector<float> Input::getCVList(int index) {
	std::vector<float> ret;
	Input::inputMutex.lock();
	ret = std::vector<float>(lastCV[index]);
	Input::inputMutex.unlock();
	return ret;
}

int Input::smooth(int in, int PrevVal) {
	int margin = PrevVal * 0.000001; //  get 2% of the raw value.  Tune for lowest non-jitter value.

	/*
	 * Next we add (or subtract...) the 'standard' fixed value to the previous reading. (PotPrevVal needs to be declared outside the function so it persists.)
	 * Here's the twist: Since the jitter seems to be worse at high raw vals, we also add/subtract the 2% of total raw. Insignificantat on low
	 * raw vals, but enough to remove the jitter at raw >900 without wrecking linearity or adding 'lag', or slowing down the loop, etc.
	 */
	if (in > PrevVal + (3 + margin) || in < PrevVal - (3 + margin)) { // a 'real' change in value? Tune the two numeric values for best results

		//average last 2 values ofr smoothing
		in = (PrevVal + in) / 2.0;
		//PotPrevVal = in; // store valid raw val  for next comparison
		return in;
	}
	return -1;
}

//==============================================================================

#endif
