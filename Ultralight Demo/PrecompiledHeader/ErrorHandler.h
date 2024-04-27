#pragma once
#include <string>

typedef long LONG;
typedef LONG HRESULT;

class ErrorHandler
{
public:
	static void LogCriticalError(const std::string msg);
	static void LogCriticalError(HRESULT hr, 
								 const std::string msg, 
								 const std::string file, 
								 const std::string function, 
								 int line);
	static void LogCriticalError(const std::string msg,
								 const std::string file,
								 const std::string function,
								 int line);
	static void LogFatalError(HRESULT hr, 
							  const std::string msg, 
							  const std::string file, 
							  const std::string function, 
							  int line);
	static void LogFatalError(const std::string msg, 
							  const std::string file, 
							  const std::string function, 
							  int line);
private:
	static std::string BuildErrorMessage(HRESULT hr, 
										 const std::string msg, 
										 const std::string file, 
										 const std::string function, 
										 int line);
	static std::string BuildErrorMessage(const std::string msg, 
										 const std::string file, 
										 const std::string function, 
										 int line);
};

#define ReturnFalseIfFail( hr, msg ) if( FAILED( hr ) ) { ErrorHandler::LogCriticalError( hr, msg, __FILE__, __FUNCTION__, __LINE__ ); DebugBreak(); return false; }
#define FatalErrorIfFalse( bValue, msg ) if( bValue == false ) { ErrorHandler::LogFatalError(msg, __FILE__, __FUNCTION__, __LINE__ ); }
#define FatalErrorIfFail( hr, msg ) if( FAILED( hr ) ) { ErrorHandler::LogFatalError( hr, msg, __FILE__, __FUNCTION__, __LINE__ ); }
#define FatalError(msg) ErrorHandler::LogFatalError(msg, __FILE__, __FUNCTION__, __LINE__ ); 
#define CriticalError(msg) ErrorHandler::LogCriticalError(msg, __FILE__, __FUNCTION__, __LINE__ ); 
