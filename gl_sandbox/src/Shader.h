#pragma once

#include <string>
#include <vector>

enum class ShaderType
{
	None = 0,
	Vertex,
	Fragment
};

class Shader
{
public:
	Shader(std::string&& _path, ShaderType _type) 
		: file_path(std::move(_path)), type(_type)  {}

	unsigned int id = 0;
	std::string file_path;
	ShaderType type;
};

class ShaderProgram
{
public:
	template<class ... Shaders>
	ShaderProgram(Shaders ... s)
	{
		create_program();

		std::vector<Shader> shaders = { s ... };

		for (int i = 0; i < shaders.size(); ++i)
		{
			std::string src = load_shader(shaders[i]);

			create_shader(shaders[i], src);

			compile_shader(shaders[i].id);

			attach_shader(shaders[i].id);
		}
	}

	void bind() const;
	void unbind() const;

private:
	void create_program();
	std::string load_shader(const Shader& s);
	void create_shader(Shader& s, const std::string& src);
	void compile_shader(unsigned int id);
	void attach_shader(unsigned int id);
	void link();

	unsigned int m_program_id;
};