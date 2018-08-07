#ifndef FILEWATCHER_CPP
#define FILEWATCHER_CPP

#include "fileWatcher.h"
#include <iostream>
#include <unistd.h>

#include <string.h>
#include <fstream>
#include <fcntl.h>


FileWatcher::FileWatcher(std::string directory, std::function<void(void)> onModify) {
	//set callbacks
	modifyCallback = onModify;
	// Monitor Shader Changes
	fd = inotify_init();
	/*checking for error*/
	if ( fd < 0 ) {
		perror( "inotify_init" );
	}

	wd = inotify_add_watch( fd, directory.c_str(), IN_CREATE | IN_MODIFY | IN_DELETE );

	if ( wd < 0 ) {
		std::cout << "File Watcher Error for: " << directory << std::endl;
	}
	else{
		std::cout << "Watching directory: " << directory << std::endl;
	}

	//start watcher thread
	threadRunning = true;
	watcherThread = std::thread(&FileWatcher::watch, this);
}

FileWatcher::~FileWatcher() {
	if (threadRunning) {
		threadRunning = false;
		watcherThread.join();
	}

	/*removing the “/tmp” directory from the watch list.*/
	inotify_rm_watch( fd, wd );

	/*closing the INOTIFY instance*/
	close( fd );
}

void FileWatcher::watch() {
	
	while (1) {
		/*read to determine the event change happens on “/tmp” directory. Actually this read blocks until the change event occurs*/
		int length = read( fd, buffer, EVENT_BUF_LEN );
		int i = 0;

		/*checking for error*/
		if ( length < 0 ) {
			perror( "read" );
		}

		/*actually read return the list of change events happens. Here, read the change event one by one and process it accordingly.*/
		while ( i < length ) {
			struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
			if ( event->len ) {
				if ( event->mask & IN_CREATE ) {
					if ( event->mask & IN_ISDIR ) {
						//printf( "New directory %s created.\n", event->name );
					}
					else {
						//printf( "New file %s created.\n", event->name );
					}
				}
				else if ( event->mask & IN_MODIFY ) {
					if ( event->mask & IN_ISDIR ) {
						//printf( "Directory %s modified.\n", event->name );
					}
					else {
						//printf( "File %s modified.\n", event->name );
						if (modifyCallback){
							modifyCallback();
						}
					}
				}
			}
			i += EVENT_SIZE + event->len;
		}
	}
	threadRunning = false;
}


#endif
