#pragma once

#include "pch.h"

#ifdef DEBUG
	
#define INFO(x) printf(x)
#define ASSERT(x) \
		if(!x) \
			__debugbreak();

#else

#define INFO(x)
#define ASSERT(x)

#endif

