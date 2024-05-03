#pragma once

class MyLogger
{
public:
	static void LogInfo(const char* msg);
};

#ifdef MYLOGGER_LOG_INFO
#define LOGINFO(msg) MyLogger::LogInfo(msg)
#else
#define LOGINFO //
#endif