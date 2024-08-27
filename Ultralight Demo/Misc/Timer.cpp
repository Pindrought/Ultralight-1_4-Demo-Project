#include "PCH.h"
#include "Timer.h"

Timer::Timer()
{
	m_Start = std::chrono::high_resolution_clock::now();
	m_Stop = std::chrono::high_resolution_clock::now();
}

double Timer::GetMilisecondsElapsed()
{
	if (m_IsRunning)
	{
		auto elapsed = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - m_Start);
		return elapsed.count();
	}
	else
	{
		auto elapsed = std::chrono::duration<double, std::milli>(m_Stop - m_Start);
		return elapsed.count();
	}
}

void Timer::Restart()
{
	m_IsRunning = true;
	m_Start = std::chrono::high_resolution_clock::now();
}

bool Timer::Stop()
{
	if (!m_IsRunning)
		return false;
	else
	{
		m_Stop = std::chrono::high_resolution_clock::now();
		m_IsRunning = false;
		return true;
	}
}

bool Timer::Start()
{
	if (m_IsRunning)
	{
		return false;
	}
	else
	{
		m_Start = std::chrono::high_resolution_clock::now();
		m_IsRunning = true;
		return true;
	}
}

void Timer::AdvanceMiliseconds(long ms)
{
	m_Start += std::chrono::milliseconds(ms);
}

std::string Timer::ToString()
{
	return std::to_string(GetMilisecondsElapsed());
}
