#include "pch.h"
#include "Renderer.h"

#include "GLError.h"
#include "events/EventList.h"

#include <glad/glad.h>

#define DEG_TO_RAD(x) (x / 180.f) * 3.1415f

Renderer::Renderer(int width, int height)
	: m_screen_width(width), m_screen_height(height),
	m_lava_texure("resources/textures/lava.png"),
	m_skybox_texture("resources/skyboxes/above_the_clouds/")
{
	GL_CALL(glViewport(0, 0, width, height));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glFrontFace(GL_CCW);

	m_camera = std::unique_ptr<Camera>(new Camera(width, height));

	// bind events
	//EventList::e_camera_move.bind_function(std::bind(&Renderer::update_camera_pos, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	unsigned int a_position = 0, a_tex_coord = 1;

	float scaling_factor = 1.0f / tanf(DEG_TO_RAD(45.f) * 0.5f);
	float aspect_ratio = (float)m_screen_height / (float)m_screen_width;

	float q = m_far / (m_far - m_near);

	// create projection matrix
	m_perspective.set(
		aspect_ratio * scaling_factor, 0.f, 0.f, 0.f,
		0.f, scaling_factor, 0.f, 0.f,
		0.f, 0.f, q, 1.f,
		0.f, 0.f, -m_near * q, 0.f
	);

	m_orthographic.set(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f / (m_far - 0.1f), -m_near / (m_far - m_near),
		0.f, 0.f, 0.f, 1.f
	);

	// move square
	math::Mat4 square_move(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.1f, -4.f, 1.f 
	);

	float skybox_verts[] =
	{
		//   Coordinates
		-1.0f, -1.0f,  1.0f,	//        7--------6
		 1.0f, -1.0f,  1.0f,	//       /|       /|
		 1.0f, -1.0f, -1.0f,	//      4--------5 |
		-1.0f, -1.0f, -1.0f,	//      | |      | | 
		-1.0f,  1.0f,  1.0f,	//      | 3------|-2
		 1.0f,  1.0f,  1.0f,	//      |/       |/
		 1.0f,  1.0f, -1.0f,	//      0--------1
		-1.0f,  1.0f, -1.0f
	};

	unsigned int skybox_indices[] =
	{
		// Right
		1, 2, 6,
		6, 5, 1,
		// Left
		0, 4, 7,
		7, 3, 0,
		// Top
		4, 5, 6,
		6, 7, 4,
		// Bottom
		0, 3, 2,
		2, 1, 0,
		// Back
		0, 1, 5,
		5, 4, 0,
		// Front
		3, 7, 6,
		6, 2, 3
	};

	m_skybox_va.reset(new VertexArray());

	m_skybox_vb = VertexBuffer();
	m_skybox_vb.add_data(skybox_verts, sizeof(skybox_verts));

	BufferLayout sb_layout = { {0, 3, GL_FLOAT, false} };

	m_skybox_va->set_layout(m_skybox_vb, sb_layout);

	m_skybox_ib.reset(new IndexBuffer(skybox_indices, sizeof(skybox_indices)));

	m_skybox_shader.reset(new ShaderProgram(
		Shader("resources/shaders/skybox/skybox_vertex.shader", ShaderType::Vertex),
		Shader("resources/shaders/skybox/skybox_fragment.shader", ShaderType::Fragment)
	));

	m_skybox_shader->set_uniform_mat4f("u_projection", m_perspective * m_orthographic);

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

	m_cube_va.reset(new VertexArray());

	m_cube_vb = VertexBuffer();
	m_cube_vb.add_data(vertices, sizeof(vertices));

	BufferLayout layout = {
		{a_position, 3, GL_FLOAT, false},
		{a_tex_coord, 2, GL_FLOAT, false}
		//{a_colour, 4, GL_FLOAT, false}
	};

	m_cube_va->set_layout(m_cube_vb, layout);

	m_cube_ib.reset(new IndexBuffer(indices, sizeof(indices)));

	m_cube_shader.reset(new ShaderProgram(
		Shader("resources/shaders/texture2D/texture2D_vertex.shader", ShaderType::Vertex),
		Shader("resources/shaders/texture2D/texture2D_fragment.shader", ShaderType::Fragment)
	));

	m_cube_shader->set_uniform_mat4f("u_translate", square_move);
	m_cube_shader->set_uniform_mat4f("u_proj", m_perspective * m_orthographic);
}

void Renderer::update(float elpased_time)
{
	m_camera->update(elpased_time);

	m_cube_shader->set_uniform_mat4f("u_view", m_camera->camera_look_at());
	m_skybox_shader->set_uniform_mat4f("u_view", m_camera->look_at_no_translate());

	draw();
}

void Renderer::draw()
{
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	GL_CALL(glDepthMask(GL_FALSE));
	m_skybox_va->bind();
	m_skybox_ib->bind();
	m_skybox_shader->bind();
	m_skybox_texture.bind();
	GL_CALL(glDrawElements(GL_TRIANGLES, m_skybox_ib->get_count(), GL_UNSIGNED_INT, nullptr));

	GL_CALL(glDepthMask(GL_TRUE));
	m_cube_va->bind();
	m_cube_ib->bind();
	m_lava_texure.bind(0);
	m_cube_shader->bind();
	GL_CALL(glDrawElements(GL_TRIANGLES, m_cube_ib->get_count(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::reset_view()
{
	m_camera->reset();
}


