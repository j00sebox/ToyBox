#pragma once

#include "Shader.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Texture.h"

#include "Camera.h"

class Renderer
{
public:
	Renderer();
	void update(float elpased_time);
	
private:
	void draw();

	std::unique_ptr<Camera> m_camera = std::make_unique<Camera>();

	std::shared_ptr<VertexArray> m_va;
	std::shared_ptr<ShaderProgram> m_shader;
	std::shared_ptr<IndexBuffer> m_ib;

	VertexBuffer m_vb;
	Texture2D m_lava_texure;
};

