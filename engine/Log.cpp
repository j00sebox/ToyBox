#include "pch.h"
#include "Log.h"

#ifdef PLATFORM_LINUX
#include <csignal>
#endif

void stop_program_()
{
#ifdef PLATFORM_WINDOWS
	__debugbreak();
#else
	raise(SIGTRAP);
#endif
}