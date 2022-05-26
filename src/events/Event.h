#pragma once

#include <functional>

#include "Log.h"

template<typename ... Args>
class Event
{
public:
	Event() = default;

	void bind_function(std::function<void(Args...)> func)
	{
		m_function = func;
	}

	void execute_function(Args... args)
	{
		if (!m_function)
			fatal("No function was bound!\n");

		m_function(args...);
	}

private:
	std::function<void(Args...)> m_function;
};