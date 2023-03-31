#pragma once

#include "renderer/Shader.h"
#include "renderer/VertexArray.h"
#include "renderer/Texture.h"

class Skybox
{
public:
	explicit Skybox(const std::string& texture_path);
	Skybox(Skybox&& sb) noexcept;

	void draw() const;
    void bind() const;
    void unbind() const;
    [[nodiscard]] const unsigned int get_indices() const { return m_indices_count; }
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

