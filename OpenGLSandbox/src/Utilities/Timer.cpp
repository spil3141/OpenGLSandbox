#include "Timer.h"
#include <string>

Timer::Timer()
{
	m_StartTimepoint = std::chrono::steady_clock::now();
}

Timer::~Timer()
{
}

std::string Timer::GetWindowTitle()
{
	auto endTimepoint = std::chrono::steady_clock::now();
	auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch();
	return std::string("frametime: ") + std::to_string(elapsedTime.count() * 0.001) + std::string(" m/s");
}


 std::chrono::milliseconds MicroToMilliConversion(std::chrono::microseconds micro)
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(micro);
}