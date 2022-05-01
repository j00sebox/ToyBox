#include "pch.h"
#include "PointLight.h"

#include "GLError.h"
#include "Buffer.h"
#include "Shader.h"

#include <glad/glad.h>

PointLight::PointLight(float radius)
	: m_radius(radius)
{
	m_position = { 0.f, 0.f, 0.f };
	m_colour = { { 1.f, 1.f, 1.f }, 1.f }; // temporary

	std::vector<float> light_vertices =
	{
		-0.1f, -0.1f,  0.1f,
		-0.1f, -0.1f, -0.1f,
		 0.1f, -0.1f, -0.1f,
		 0.1f, -0.1f,  0.1f,
		-0.1f,  0.1f,  0.1f,
		-0.1f,  0.1f, -0.1f,
		 0.1f,  0.1f, -0.1f,
		 0.1f,  0.1f,  0.1f
	};

	std::vector<unsigned int> light_indices =
	{
		0, 1, 2,
		0, 2, 3,
		0, 4, 7,
		0, 7, 3,
		3, 7, 6,
		3, 6, 2,
		2, 6, 5,
		2, 5, 1,
		1, 5, 4,
		1, 4, 0,
		4, 5, 6,
		4, 6, 7
	};

	m_indices_count = light_indices.size();

	m_va.bind();

	VertexBuffer vb(light_vertices);
	IndexBuffer ib(light_indices);

	BufferLayout layout = {
		{ 0, 3, GL_FLOAT, false }
	};

	m_va.set_layout(vb, layout);

	m_shader = std::make_shared<ShaderProgram>(
		ShaderProgram(
			Shader("resources/shaders/point_light/point_light_vertex.shader", ShaderType::Vertex),
			Shader("resources/shaders/point_light/point_light_fragment.shader", ShaderType::Fragment)
		)
	);

	m_va.unbind();
	ib.unbind();
	vb.unbind();
}

void PointLight::draw() const
{
	m_va.bind();
	m_shader->bind();
	GL_CALL(glDrawElements(GL_TRIANGLES, m_indices_count, GL_UNSIGNED_INT, nullptr));
}

void PointLight::set_postion(const mathz::Vec3 position)
{
	m_position = position;
}

void PointLight::set_colour(const mathz::Vec4 colour)
{
	m_colour = colour;
}
