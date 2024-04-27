#include "ErrorHandler.h"
#include "StringConverter.h"
#include <Windows.h>
#include <comdef.h>

void ErrorHandler::LogCriticalError(const std::string msg)
{
	MessageBoxA(NULL, msg.c_str(), "Critical Error", 0);
}

void ErrorHandler::LogCriticalError(HRESULT hr,
									const std::string msg,
									const std::string file,
									const std::string function,
									int line)
{
	std::string errorMsg = BuildErrorMessage(hr, msg, file, function, line);
	MessageBoxA(NULL, errorMsg.c_str(), "Critical Error", 0);
}

void ErrorHandler::LogCriticalError(const std::string msg, const std::string file, const std::string function, int line)
{
	std::string errorMsg = BuildErrorMessage(msg, file, function, line);
	MessageBoxA(NULL, errorMsg.c_str(), "Critical Error", 0);
}

void ErrorHandler::LogFatalError(HRESULT hr,
								 const std::string msg,
								 const std::string file,
								 const std::string function,
								 int line)
{
	std::string errorMsg = BuildErrorMessage(hr, msg, file, function, line);
	MessageBoxA(NULL, errorMsg.c_str(), "Fatal Error", 0);
	exit(-1);
}

void ErrorHandler::LogFatalError(const std::string msg,
								 const std::string file,
								 const std::string function,
								 int line)
{
	std::string errorMsg = BuildErrorMessage(msg, file, function, line);
	MessageBoxA(NULL, errorMsg.c_str(), "Fatal Error", 0);
	exit(-1);
}

std::string ErrorHandler::BuildErrorMessage(HRESULT hr, 
											const std::string msg, 
											const std::string file, 
											const std::string function, 
											int line)
{
	std::string errorMsg = msg + "\n";
	errorMsg += "File: ";
	errorMsg += file + "\n";

	_com_error comError(hr);
	errorMsg += StringConverter::ws2s(comError.ErrorMessage());

	errorMsg += "\nFunction:";
	errorMsg += function;
	errorMsg += "\nLine:";
	errorMsg += std::to_string(line);

	return errorMsg;
}

std::string ErrorHandler::BuildErrorMessage(const std::string msg, 
											const std::string file, 
											const std::string function, 
											int line)
{
	std::string errorMsg = msg + "\n";
	errorMsg += "File: ";
	errorMsg += file;
	errorMsg += "\nFunction:";
	errorMsg += function;
	errorMsg += "\nLine:";
	errorMsg += std::to_string(line);
	return errorMsg;
}
