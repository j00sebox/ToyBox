#pragma once

#include "Log.h"

const char* gl_get_err_str(unsigned int err);

bool gl_check_error(const char* func_name, const char* file_name, int line);

void gl_clear_errors();

#ifdef DEBUG

#define GL_CALL(x) \
	gl_clear_errors(); \
	x; \
	ASSERT(gl_check_error(#x, __FILE__, __LINE__))

#else

#define GL_CALL(x) x

#endif // DEBUG


