#pragma once

#include "Log.h"

enum class ShaderType
{
	None = 0,
	Vertex,
	Fragment
};

struct Shader
{
	Shader(const char* _path, ShaderType _type) 
		: file_path(std::move(_path)), type(_type)  {}

	unsigned int id = 0;
	std::string file_path;
	ShaderType type;
};

template<typename T>
concept is_shader = std::convertible_to<T, Shader>;

class ShaderProgram
{
public:
	template<is_shader ... Shaders>
	ShaderProgram(Shaders ... s)
	{
		create_program();

		m_shaders = { s ... };

		for (int i = 0; i < m_shaders.size(); ++i)
		{
			std::string src = load_shader(m_shaders[i]);

			create_shader(m_shaders[i], src);

			compile_shader(m_shaders[i].id);

			attach_shader(m_shaders[i].id);
		}

		link();
		delete_shaders();
	}

	~ShaderProgram();

	void bind() const;
	void unbind() const;

private:
	void create_program();
	std::string load_shader(const Shader& s) const;
	void create_shader(Shader& s, const std::string& src) const;
	void compile_shader(unsigned int id) const;
	void attach_shader(unsigned int id) const;
	void link();
	void delete_shaders(); // also does a detach

	unsigned int m_program_id;
	std::vector<Shader> m_shaders;
};