#pragma once
#include <PCH.h>

class LoggerDefault : public ultralight::Logger
{
public:
    LoggerDefault(std::string logFilePath = "");
    void LogMessage(ultralight::LogLevel logLevel, const ultralight::String& message) override;
private:
    bool m_LogToTextFile = false;
    std::string m_LogFilePath = "";
};