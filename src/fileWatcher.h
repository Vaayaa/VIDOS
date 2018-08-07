#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <thread>
#include <mutex>
#include <sys/inotify.h>
#include <functional>
#include <string>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

class FileWatcher
{
public:
	FileWatcher(std::string directory, std::function<void(void)> onModify);
	~FileWatcher();
private:
	void watch();
	int fd;
	int wd;
	std::thread watcherThread;
	char buffer[EVENT_BUF_LEN];
	bool threadRunning = false;

	std::function<void(void)> modifyCallback;
};


#endif
