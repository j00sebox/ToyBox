#include "pch.h"
#include "Geometry.h"

#include "GLError.h"
#include "GLTFLoader.h"

#include <glad/glad.h>

Object::Object(const std::string& file_path)
{
	if (file_path.empty())
	{
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

		m_ib.reset(new IndexBuffer(indices, sizeof(indices)));

		BufferLayout layout = {
			{0, 3, GL_FLOAT, false},
			{1, 2, GL_FLOAT, false}
		};

		m_va->set_layout(m_vb, layout);
	}
	else
	{
		GLTFLoader loader(file_path.c_str());

		std::vector<float> positions = loader.get_positions();
		std::vector<float> tex_coords = loader.get_tex_coords();

		std::vector<float> vertices;

		std::vector<unsigned int> indices = loader.get_indices();
	}
}

void Object::draw() const
{
	m_va->bind();
	m_ib->bind();
	GL_CALL(glDrawElements(GL_TRIANGLES, m_ib->get_count(), GL_UNSIGNED_INT, nullptr));
}

void Object::translate(const math::Vec3& pos)
{
	math::Mat4 t;
	t(3, 0) = pos.x;	t(3, 1) = pos.y;	t(3, 2) = pos.z;

	m_model_transform *= t;
}
