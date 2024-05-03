#include "PCH.h"

void MyLogger::LogInfo(const char* msg)
{
	OutputDebugStringA("INFO::");
	OutputDebugStringA(msg);
	OutputDebugStringA("\n");
}
