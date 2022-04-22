#include "pch.h"
#include "GLError.h"

#include <glad/glad.h>

const char* gl_get_err_str(unsigned int err)
{
	switch (err)
	{
	case GL_NO_ERROR:          return "No error";
	case GL_INVALID_ENUM:      return "Invalid enum";
	case GL_INVALID_VALUE:     return "Invalid value";
	case GL_INVALID_OPERATION: return "Invalid operation";
	case GL_STACK_OVERFLOW:    return "Stack overflow";
	case GL_STACK_UNDERFLOW:   return "Stack underflow";
	case GL_OUT_OF_MEMORY:     return "Out of memory";
	default:                   return "Unknown error";
	}
}

bool gl_check_error(const char* func_name, const char* file_name, int line)
{
	while (GLenum err = glGetError())
	{
		if (GL_NO_ERROR == err)
			break;

		printf("[%i] Error: %s at function: %s in { file: %s, line %i }\n", err, gl_get_err_str(err), func_name, file_name, line);

		return false;
	}

	return true;
}

void gl_clear_errors()
{
	while (glGetError());
}