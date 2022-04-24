#include "pch.h"
#include "Renderer.h"

#include "GLError.h"

#include <glad/glad.h>

Renderer::Renderer()
	: m_lava_texure("res/textures/lava.png")
{
	unsigned int a_position = 0, a_tex_coord = 1;

	// move square up 0.25 units
	math::Mat4 square_move(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.5f, 0.f, 1.f
	);

	float vertices[] =
	{
	   -0.5f, -0.5f, 0.f, 0.f, 0.f,		// 1.f, 0.f, 0.f, 1.f,
		0.5f,  0.5f, 0.f, 1.f, 1.f,  	// 0.f, 0.f, 1.f, 1.f,
	   -0.5f,  0.5f, 0.f, 0.f, 1.f,		// 0.f, 1.f, 0.f, 1.f,
		0.5f, -0.5f, 0.f, 1.f, 0.f		// 0.f, 1.f, 0.f, 1.f,
	};

	unsigned int indices[] = {
		0, 1, 2,
		0, 3, 1
	};

	m_va.reset(new VertexArray());

	m_vb = VertexBuffer();
	m_vb.add_data(vertices, sizeof(vertices));

	BufferLayout layout = {
		{a_position, 3, GL_FLOAT, false},
		{a_tex_coord, 2, GL_FLOAT, false}
		//{a_colour, 4, GL_FLOAT, false}
	};

	m_va->set_layout(m_vb, layout);

	m_ib.reset(new IndexBuffer(indices, sizeof(indices)));

	m_shader.reset(new ShaderProgram(
		Shader("res/shaders/texture2D/texture2D_vertex.shader", ShaderType::Vertex),
		Shader("res/shaders/texture2D/texture2D_fragment.shader", ShaderType::Fragment)
	));


	m_shader->set_uniform_mat4f("u_translate", square_move);
}

void Renderer::update(float elpased_time)
{
	m_lava_texure.bind(0);

	draw();
}

void Renderer::draw()
{
	m_va->bind();
	m_ib->bind();
	m_shader->bind();
	GL_CALL(glDrawElements(GL_TRIANGLES, m_ib->get_count(), GL_UNSIGNED_INT, nullptr));
}
