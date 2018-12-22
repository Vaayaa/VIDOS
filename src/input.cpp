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
#include "oscListener.h"

#define MUX_ADDRESS_PINA 0
#define MUX_ADDRESS_PINB 2
#define MUX_ADDRESS_PINC 3


Input::Input() {
	onButton = 0;
	//go through input possibilities
	//try to use serial
	if(setupSerial()){
		std::cout << "Using Serial (Arduino) for Input." << std::endl;
	}
	//try to use Hardware ADC
	else if(!useOSC && setupADC()){
		std::cout << "Using MCP3008 for Input." << std::endl;

		//Setup Muxing pins
		pinMode (MUX_ADDRESS_PINA, OUTPUT) ;
		pinMode (MUX_ADDRESS_PINB, OUTPUT) ;
		pinMode (MUX_ADDRESS_PINC, OUTPUT) ;
		//Set Pulldowns
		pullUpDnControl (MUX_ADDRESS_PINA, PUD_DOWN);
		pullUpDnControl (MUX_ADDRESS_PINB, PUD_DOWN);
		pullUpDnControl (MUX_ADDRESS_PINC, PUD_DOWN);
	}
	//setup OSC control
	else{
		setupOSC();
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

bool Input::setupOSC(){
    threadRunning = true;
    inputThread = std::thread(&Input::readOSC, this);
    std::cout << "Using OSC." << std::endl;
    return true;
}

bool Input::readOSC(){
	MyPacketListener listener(this);
    UdpListeningReceiveSocket s(
            IpEndpointName( IpEndpointName::ANY_ADDRESS, PORT ),
            &listener );
    std::cout << "listening for input on port " << PORT << "...\n";
    s.Run();
    return true;
}

bool Input::setupADC() {
	//initialize wiringPi
	if (wiringPiSetup() == -1)
	{
		std::cout << "Input Error: WiringPi setup failure" << std::endl;
		return false;
	}
	if(mcp3004Setup(BASE, SPI_CHAN)){
		//start input thread
		threadRunning = true;
		inputThread = std::thread(&Input::readADC, this);
		return true;
	}
	return false;
}

bool Input::readADC() {
	//counter for multiplexing ( this should give slightly more priority to sampling CV in theory)
	int muxCounter = 0;
	while (threadRunning) {
		//read mcp3008
		for ( int chan = 0; chan < 8 ; chan++)
		{
			//Read in all the Pots
			//Channel 0 is Multiplexed with CD4051BE 
			if( chan  == 0 ){
				// go through one of the 8 addresses of the mutliplexer via counter
					// mask out individual bits and set address pins correctly
				digitalWrite(MUX_ADDRESS_PINA, (bool)(muxCounter&1));
				digitalWrite(MUX_ADDRESS_PINB, (bool)(muxCounter&2));
				digitalWrite(MUX_ADDRESS_PINC, (bool)(muxCounter&4));

				int val = analogRead( BASE + chan);
				// if(muxCounter  == 2 ){
				// 	std::cout<< "muxCounter: "<< muxCounter << " MASKS:" <<  (bool)(muxCounter&1) << " " << (bool)(muxCounter&2) << " " << (bool)(muxCounter&4) << std::endl;
				// 	std::cout<< "Pot " << muxCounter << " : " << val << std::endl;
				// }

			}

			else{
				//read in ADC values
				int val = analogRead( BASE + chan);
				int smoothVal = smooth(val, lastSmoothCV[chan]);
				if(smoothVal > -1){
					lastSmoothCV[chan] = val;
					setCV(chan, smoothVal / 1024.0);

					// if( chan == 0)
					//printf("%d: %d lst: %d \n", chan, val, lastSmoothCV[chan]);
				}
			}
		}

		//advance and reset muxcounter
		++muxCounter;
		if(muxCounter > 7) { muxCounter = 0;}

		//read button pin
		// int val = digitalRead(29); //Physical Pin 40, run "gpio readall" for pin details
		// bool pressedDown = false;
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

		//readSwitches();
	}

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
	// if(index == 2)
	// std::cout << "Setting CV " << index << ": "<< val<< std::endl;

	//~ smoothVal = smooth(val, lastCV[chan-3]);
	//~ if(smoothVal != -1){
	//~ inputMutex.lock();
	//~ cvIn[chan-3] = val/1024.0;
	//~ inputMutex.unlock();
	//lastCV[chan-3] = val;
	//~ }
	if (index > 7 ){ //dont support anyting above 7 for now

		if(index >= 100){ //this is a switch (probably rewrite this with a map :/ )
			setSwitch(index - 100, (int)val );
		}
		return;
	}
	inputMutex.lock();
	clock_t readtime = clock();
	cvIn[index] = val;
	lastCV[index].push_back(val);
	lastCV[index].erase(lastCV[index].begin());
	inputMutex.unlock();
	//save time when the read was performed
	lastCVRead[index] = (float) readtime / (CLOCKS_PER_SEC);
}

float Input::getCV(int i) {
	float ret;
	Input::inputMutex.lock();
	ret = cvIn[i];
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

int Input::getSwitch(int i){
	float ret = 0.0;
	if(i < SWITCH_COUNT){
		Input::inputMutex.lock();
		ret = switches[i];
		Input::inputMutex.unlock();
	}
	return ret;
}

void Input::setSwitch(int index, int switchPos){
	// 0 is off (center) 1 is up 2 is down
	inputMutex.lock();
	switches[index] = switchPos;
	// std::cout<<index << ": " << switchPos << std::endl;
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

 bool Input::readSwitches(){
	//read switches
 	set3PosSwitch(0, 4, 5);
	set3PosSwitch(1, 0, 2);
	set3PosSwitch(2, 22, 23);
	return true;
}

#endif
