#include <PCH.h>
#include "LoggerDefault.h"

LoggerDefault::LoggerDefault(std::string logFilePath)
{
	if (logFilePath != "")
	{
		m_LogFilePath = logFilePath;
		m_LogToTextFile = true;
	}
}

void LoggerDefault::LogMessage(ul::LogLevel logLevel, const ul::String& message)
{
	if (m_LogToTextFile)
	{
		std::ofstream log(m_LogFilePath.c_str(), std::ios::app);
		if (log.is_open())
		{
			log << "[";
			switch (logLevel)
			{
			case ul::LogLevel::Info:
				log << "Info]: ";
				break;
			case ul::LogLevel::Warning:
				log << "Warning]: ";
				break;
			case ul::LogLevel::Error:
				log << "Error]: ";
				break;
			}
			log << message.utf8().data() << std::endl;
			log.close();
		}
		else
		{
			LOGINFO("Failed to open log file.\n");
		}
	}

	//TODO: This probably shouldn't be here.
	if (logLevel == ul::LogLevel::Error ||
		logLevel == ul::LogLevel::Warning)
		ErrorHandler::LogCriticalError(ul::String(message).utf8().data());
}