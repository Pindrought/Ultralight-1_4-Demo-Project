#pragma once
#include <PCH.h>

class Timer
{
public:
	Timer();
	double GetMilisecondsElapsed();
	void Restart();
	bool Stop();
	bool Start();
	void AdvanceMiliseconds(long ms);
	std::string ToString();
private:
	bool m_IsRunning = false;
#ifdef _WIN32
	std::chrono::time_point<std::chrono::steady_clock> m_Start;
	std::chrono::time_point<std::chrono::steady_clock> m_Stop;
#else
	std::chrono::time_point<std::chrono::system_clock> m_Start;
	std::chrono::time_point<std::chrono::system_clock> m_Stop;
#endif
};