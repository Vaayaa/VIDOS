#include <thread>
#include <mutex>
#include <functional>
#include <vector>

#define CV_LIST_SIZE 400
#define CV_COUNT 3

class Input
{
public:
	Input();
	~Input();
	
	void update();
	void addButtonCallback(std::function<void(bool)> buttonCallback);
	void setCV (int index, float val);
	std::vector<float> getCVList(int index);
	float getCV(int i);
	float getPot(int i);

	
	std::thread inputThread;
	std::mutex inputMutex;
  
private:
	//options
	bool useSerial = false;
	
	int serialFd;
	float cvIn[3] = {0};
	float potIn[3] = {1.};
	int lastPot[3] = {0};
	std::vector<float> lastCV[3] = {std::vector<float>(CV_LIST_SIZE, 0.0), std::vector<float>(CV_LIST_SIZE, 0.0), std::vector<float>(CV_LIST_SIZE, 0.0)};
	float lastCVRead[3] = {0.};
	int buttonIn = 0;
	
	std::function<void(bool)> onButton;
	
	bool setupSerial();
	bool readSerial();
	bool setupADC();
	bool readADC();
	static int smooth(int in, int PrevVal);
	
	bool threadRunning = false;
	

};
