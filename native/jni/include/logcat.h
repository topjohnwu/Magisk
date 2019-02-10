#pragma once

#include <string>
#include <BlockingQueue.h>

enum logcat_event {
	HIDE_EVENT,
	LOG_EVENT
};

extern bool logcat_started;

BlockingQueue<std::string> &start_logging(logcat_event event);
void stop_logging(logcat_event event);
bool start_logcat();
