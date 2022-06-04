#include "pch.h"
#include "Log.h"

#ifdef PLATFORM_LINUX
#include <csignal>
#endif

void __stop_program__()
{
#ifdef PLATFORM_WINDOWS
	__debugbreak();
#else
	raise(SIGTRAP);
#endif
}