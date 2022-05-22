#pragma once

#include "Shader.h"
#include "VertexArray.h"
#include "Texture.h"

class Skybox
{
public:
	Skybox(const std::string& texture_path);
	Skybox(Skybox&& sb) noexcept;

	void draw() const;
	void attach_shader_program(ShaderProgram&& sp);
	[[nodiscard]] const std::shared_ptr<ShaderProgram>& get_shader() const { return m_skybox_shader; }
	[[nodiscard]] const std::string& get_resource_path() const { return m_path; }

	void operator= (Skybox&& sb) noexcept;

private:
	unsigned int m_indices_count;
	VertexArray m_skybox_va;
	CubeMap m_skybox_texture;
	std::shared_ptr<ShaderProgram> m_skybox_shader;

	// TODO: Think about removing this
	std::string m_path;
};

