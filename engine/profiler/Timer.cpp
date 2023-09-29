#include "pch.h"
#include "Timer.hpp"
#include "Log.hpp"

Timer::Timer()
{
	m_start = std::chrono::high_resolution_clock::now();
}

Timer::~Timer()
{
    if(m_stopped)
    {
        return;
    }
	auto duration = std::chrono::high_resolution_clock::now() - m_start;
	auto time_in_ms = (f32)std::chrono::duration_cast<std::chrono::microseconds>(duration).count() * 0.001f;
	info("Time Elapsed: {} ms\n", time_in_ms);
}

f32 Timer::stop()
{
    m_stopped = true;
    auto duration = std::chrono::high_resolution_clock::now() - m_start;
    return (f32)std::chrono::duration_cast<std::chrono::microseconds>(duration).count() * 0.001f;
}