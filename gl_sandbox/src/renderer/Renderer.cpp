#include "pch.h"
#include "Renderer.h"

#include "GLError.h"
#include "events/EventList.h"

#include <glad/glad.h>

Renderer::Renderer(int width, int height)
	: m_screen_width(width), m_screen_height(height),
	m_lava_texure("res/textures/lava.png")
{
	GL_CALL(glViewport(0, 0, width, height));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// bind events
	//EventList::e_camera_move.bind_function(std::bind(&Renderer::update_camera_pos, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	
	m_camera->set_pos({ 0.f, 0.f, 0.f });

	unsigned int a_position = 0, a_tex_coord = 1;

	float scaling_factor = 1.0f / tanf(90.f * 0.5f);
	float aspect_ratio = (float)m_screen_height / (float)m_screen_width;

	float q = 1000.f / (1000.f - 0.1f);

	// create projection matrix
	m_perspective.set(
		aspect_ratio * scaling_factor, 0.f, 0.f, 0.f,
		0.f, scaling_factor, 0.f, 0.f,
		0.f, 0.f, q, 1.f,
		0.f, 0.f, -0.1f * q, 0.f
	);

	// move square
	math::Mat4 square_move(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.1f, 0.5f, 1.f
	);

	float vertices[] =
	{
	   -0.5f, -0.5f, 0.f, 0.f, 0.f,	
		0.5f,  0.5f, 0.f, 1.f, 1.f,  	
	   -0.5f,  0.5f, 0.f, 0.f, 1.f,		
		0.5f, -0.5f, 0.f, 1.f, 0.f,	

	   -0.5f, -0.5f, 1.f, 0.f, 0.f,
		0.5f,  0.5f, 1.f, 1.f, 1.f,
	   -0.5f,  0.5f, 1.f, 0.f, 1.f,
		0.5f, -0.5f, 1.f, 1.f, 0.f,
	};

	unsigned int indices[] = {
		// front
		0, 1, 2,
		0, 3, 1,

		// back
		4, 5, 6,
		4, 7, 5,

		// top
		1, 6, 2,
		1, 5, 6,

		// bottom
		0, 3, 4,
		3, 7, 4,

		// left
		0, 2, 6,
		0, 6, 4,

		//right
		3, 5, 1,
		3, 5, 7
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

	m_shader->set_uniform_mat4f("u_proj", m_perspective);
	m_shader->set_uniform_mat4f("u_translate", square_move);
}

void Renderer::update(float elpased_time)
{
	m_shader->set_uniform_mat4f("u_view", m_camera->get_transform().inverse());

	m_lava_texure.bind(0);

	draw();
}

void Renderer::update_camera_pos(const math::Vec3& pos)
{
	m_camera->set_pos(m_camera->get_pos() + pos);
}

void Renderer::update_camera_pos(float x, float y, float z)
{
	math::Vec3 current_pos = m_camera->get_pos();
	m_camera->set_pos(current_pos + ( math::Vec3{ x, y, z } ));
}

void Renderer::draw()
{
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	m_va->bind();
	m_ib->bind();
	m_shader->bind();
	GL_CALL(glDrawElements(GL_TRIANGLES, m_ib->get_count(), GL_UNSIGNED_INT, nullptr));
}
