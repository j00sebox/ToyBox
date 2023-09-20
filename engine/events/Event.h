#pragma once

#include <functional>

#include "Log.hpp"

template<typename R, typename ... Args>
class Event
{
public:
	Event() = default;

    template<typename T>
	void bind(T* instance, R (T::*cb)(Args...))
	{
        this->callback = [instance, cb](Args... args) {
            return (instance->*cb)(args...);
        };
	}

	void execute(Args... args)
	{
		if (!callback)
			fatal("No function was bound!\n");

		callback(args...);
	}

private:
	std::function<R(Args...)> callback;
};