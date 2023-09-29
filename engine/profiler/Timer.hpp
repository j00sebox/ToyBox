#pragma once
#include "CommonTypes.hpp"

#include <chrono>

class Timer
{
public:
	Timer();
	~Timer();

    f32 stop();

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
    bool m_stopped = false;
};

