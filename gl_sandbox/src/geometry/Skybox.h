#pragma once

#include "Model.h"

class Skybox : public Model
{
public:
	Skybox(const std::string& texture_path);

	virtual void draw() const override;
	void attach_shader(std::shared_ptr<ShaderProgram> shader);

private:
	unsigned int m_indices_count;
	VertexArray m_skybox_va;
	CubeMap m_skybox_texture;
	std::shared_ptr<ShaderProgram> m_skybox_shader;
};

