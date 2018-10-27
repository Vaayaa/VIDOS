#ifndef INPUT_H
#define INPUT_H

#include <thread>
#include <mutex>
#include <functional>
#include <vector>

#define SWITCH_COUNT 3
#define CV_LIST_SIZE 400
#define CV_COUNT 8


class Input
{
public:
	Input();
	~Input();

	void update();
	void addButtonCallback(std::function<void(bool)> buttonCallback);
	
	std::vector<float> getCVList(int index);
	float getCV(int index);
	void setCV (int index, float val);
	int getSwitch(int index);


	std::thread inputThread;
	std::mutex inputMutex;

private:
	//options
	bool useOSC = true;

	int serialFd;
	float cvIn[CV_COUNT] = {0};
	std::vector<float> lastCV[CV_COUNT] = {std::vector<float>(CV_LIST_SIZE, 0.0), std::vector<float>(CV_LIST_SIZE, 0.0), std::vector<float>(CV_LIST_SIZE, 0.0) , std::vector<float>(CV_LIST_SIZE, 0.0) , std::vector<float>(CV_LIST_SIZE, 0.0) , std::vector<float>(CV_LIST_SIZE, 0.0) , std::vector<float>(CV_LIST_SIZE, 0.0) , std::vector<float>(CV_LIST_SIZE, 0.0) };
	float lastCVRead[CV_COUNT] = {0.};
	int lastSmoothCV[CV_COUNT] = {0};
	int buttonIn = 0;
	//3pos Switches
	int switches[SWITCH_COUNT] = {0};

	
	void set3PosSwitch(int index, int pin1, int pin2);

	std::function<void(bool)> onButton;

	bool setupOSC();
	bool readOSC();

	bool setupSerial();
	bool readSerial();

	bool readSwitches();

	bool setupADC();
	bool readADC();
	static int smooth(int in, int PrevVal);

	bool threadRunning = false;


};


#endif
