#include "Shader.h"

#include <fstream>
#include <sstream> 
#include <iostream>

#include <glad/glad.h>

GLenum get_gl_shader_type(ShaderType type)
{
	switch (type)
	{
	case ShaderType::None:
		break;
	case ShaderType::Vertex:
		return GL_VERTEX_SHADER;
		break;
	case ShaderType::Fragment:
		return GL_FRAGMENT_SHADER;
		break;
	default:
		break;
	}

	return GL_FALSE;
}

void ShaderProgram::create_program()
{
	m_program_id = glCreateProgram();
}

std::string ShaderProgram::load_shader(const Shader& s)
{
	std::string line;
	std::ifstream stream(s.file_path);

	std::stringstream ss;

	while (getline(stream, line))
	{
		ss << line << "\n";
	}

	stream.close();
	
	return ss.str();
}

void ShaderProgram::create_shader(Shader& s, const std::string& src)
{
	const char* shader_src = src.c_str();

	s.id = glCreateShader(get_gl_shader_type(s.type));
	glShaderSource(s.id, 1, &shader_src, nullptr);
}

void ShaderProgram::compile_shader(unsigned int id)
{
	glCompileShader(id);

	int success = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		int log_sz;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_sz);

		char* info_log = new char[log_sz * sizeof(char)];
		glGetShaderInfoLog(id, log_sz, nullptr, info_log);

		std::cout << info_log << "\n";

		delete[] info_log;
		glDeleteShader(id);
		__debugbreak();
	}
	else
	{
		std::cout << "Shader compilation successful!\n";
	}
}

void ShaderProgram::attach_shader(unsigned int id)
{
	glAttachShader(m_program_id, id);
}

void ShaderProgram::link()
{
	glLinkProgram(m_program_id);
}
