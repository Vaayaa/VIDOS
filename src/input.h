#include <thread>
#include <mutex>
#include <functional>
#include <list>

#define CV_LIST_SIZE 5

class Input
{
public:
	Input();
	~Input();
	
	void update();
	void addButtonCallback(std::function<void(bool)> buttonCallback);
	void setCV (int index, float val);
	std::list<float> getCVList(int index);
	float getCV(int i);
	float getPot(int i);

	
	std::thread inputThread;
	std::mutex inputMutex;
  
private:
	//options
	bool useSerial = true;
	
	int serialFd;
	float cvIn[3] = {0};
	float potIn[3] = {1.};
	int lastPot[3] = {0};
	std::list<float> lastCV[3] = {std::list<float>(CV_LIST_SIZE, 0.0)};
	float lastCVRead[3] = {0.,0.,0.};
	int buttonIn = 0;
	
	std::function<void(bool)> onButton;
	
	bool setupSerial();
	bool readSerial();
	bool setupADC();
	bool readADC();
	static int smooth(int in, int PrevVal);
	
	bool threadRunning = false;
	

};
