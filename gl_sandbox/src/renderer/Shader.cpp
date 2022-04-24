#include "pch.h"
#include "Shader.h"

#include "GLError.h"

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

ShaderProgram::~ShaderProgram()
{
	GL_CALL(glDeleteProgram(m_program_id));
}

void ShaderProgram::bind() const
{
	GL_CALL(glUseProgram(m_program_id));
}

void ShaderProgram::unbind() const
{
	GL_CALL(glUseProgram(0));
}

void ShaderProgram::create_program()
{
	GL_CALL(m_program_id = glCreateProgram());
}

std::string ShaderProgram::load_shader(const Shader& s) const
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

void ShaderProgram::create_shader(Shader& s, const std::string& src) const
{
	const char* shader_src = src.c_str();

	GL_CALL(s.id = glCreateShader(get_gl_shader_type(s.type)));
	GL_CALL(glShaderSource(s.id, 1, &shader_src, nullptr));
}

void ShaderProgram::compile_shader(unsigned int id) const
{
	GL_CALL(glCompileShader(id));

	int success = 0;
	GL_CALL(glGetShaderiv(id, GL_COMPILE_STATUS, &success));

	if (!success)
	{
		int log_sz;
		GL_CALL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_sz));

		char* info_log = new char[log_sz * sizeof(char)];
		GL_CALL(glGetShaderInfoLog(id, log_sz, nullptr, info_log));

		std::cout << info_log << "\n";

		delete[] info_log;
		GL_CALL(glDeleteShader(id));
		ASSERT(false);
	}
	else
	{
		std::cout << "Shader compilation successful!\n";
	}
}

void ShaderProgram::attach_shader(unsigned int id) const
{
	GL_CALL(glAttachShader(m_program_id, id));
}

void ShaderProgram::link()
{
	GL_CALL(glLinkProgram(m_program_id));

	int linked = 0;
	GL_CALL(glGetProgramiv(m_program_id, GL_LINK_STATUS, (int*)&linked));
	if (!linked)
	{
		int log_sz = 0;
		GL_CALL(glGetProgramiv(m_program_id, GL_INFO_LOG_LENGTH, &log_sz));

		char* info_log = new char[log_sz * sizeof(char)];
		GL_CALL(glGetProgramInfoLog(m_program_id, log_sz, nullptr, info_log));

		std::cout << info_log << "\n";

		delete[] info_log;
		delete_shaders();
		GL_CALL(glDeleteProgram(m_program_id));
		ASSERT(false);
	}
}

void ShaderProgram::delete_shaders() 
{
	if (m_shaders.size() > 0)
	{
		for (const auto& s : m_shaders)
		{
			GL_CALL(glDetachShader(m_program_id, s.id));
			GL_CALL(glDeleteShader(s.id));
		}

		m_shaders.clear();
	}	
}

int ShaderProgram::get_uniform_loaction(const std::string& name)
{
	if (m_uniform_location_cache.find(name) != m_uniform_location_cache.end())
		return m_uniform_location_cache[name];

	GL_CALL(int location = glGetUniformLocation(m_program_id, name.c_str()));

	if (location == -1)
		ASSERT(false);

	m_uniform_location_cache[name] = location;

	return location;
}

void ShaderProgram::set_uniform_1f(const std::string& name, float x)
{
	GL_CALL(glUniform1f(get_uniform_loaction(name), x));
}

void ShaderProgram::set_uniform_2f(const std::string& name, float x, float y)
{
	GL_CALL(glUniform2f(get_uniform_loaction(name), x, y));
}

void ShaderProgram::set_uniform_3f(const std::string& name, float x, float y, float z)
{
	GL_CALL(glUniform3f(get_uniform_loaction(name), x, y, z));
}

void ShaderProgram::set_uniform_4f(const std::string& name, float x, float y, float z, float w)
{
	GL_CALL(glUniform4f(get_uniform_loaction(name), x, y, z, w));
}

void ShaderProgram::set_uniform_mat4f(const std::string& name, const math::Mat4& mat)
{
	bind();
	GL_CALL(glUniformMatrix4fv(get_uniform_loaction(name), 1, GL_FALSE, &mat.mat[0][0]));
}
