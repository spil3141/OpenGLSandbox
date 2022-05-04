#pragma once
#include <chrono>
#include <iostream>
using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

static std::chrono::milliseconds MicroToMilliConversion(std::chrono::microseconds micro);

class Timer
{
public: 
	Timer();
	~Timer();
	std::string GetWindowTitle();
	
private:
	std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
};

