#include "pch.h"
#include "Geometry.h"

#include "GLError.h"
#include "GLTFLoader.h"

#include <glad/glad.h>

Object::Object(const std::string& file_path)
{
	if (file_path.empty())
	{
		std::vector<float> vertices =
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
		m_vb.add_data(vertices.data(), vertices.size() * sizeof(float));

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

		std::vector<math::Vec3> positions = floats_to_vec3(loader.get_positions());
		std::vector<math::Vec2<float>> tex_coords = floats_to_vec2(loader.get_tex_coords());

		m_texture.reset(new Texture2D(loader.get_base_color_texture()));

		std::vector<Vertex> vertices;

		for (unsigned int i = 0; i < positions.size(); i++)
		{
			vertices.push_back({
					positions[i],
					tex_coords[i]
				});
		}

		std::vector<unsigned int> indices = loader.get_indices();

		std::vector<float> verts;

		for (unsigned int i = 0; i < vertices.size(); ++i)
		{
			verts.push_back(vertices[i].positon.x);
			verts.push_back(vertices[i].positon.y);
			verts.push_back(vertices[i].positon.z);
			verts.push_back(vertices[i].st.x);
			verts.push_back(vertices[i].st.y);
		}

		m_va.reset(new VertexArray());
		
		m_vb = VertexBuffer();
		m_vb.add_data(verts.data(), verts.size() * sizeof(float));

		m_ib.reset(new IndexBuffer(indices.data(), indices.size() * sizeof(unsigned int)));

		BufferLayout layout = {
			{0, 3, GL_FLOAT, false},
			{1, 2, GL_FLOAT, false}
		};

		m_va->set_layout(m_vb, layout);
	}
}

void Object::draw() const
{
	m_va->bind();
	m_ib->bind();
	m_texture->bind(0);
	GL_CALL(glDrawElements(GL_TRIANGLES, m_ib->get_count(), GL_UNSIGNED_INT, nullptr));
}

void Object::translate(const math::Vec3& pos)
{
	math::Mat4 t;
	t(3, 0) = pos.x;	t(3, 1) = pos.y;	t(3, 2) = pos.z;

	m_model_transform *= t;
}

std::vector<math::Vec3> Object::floats_to_vec3(const std::vector<float>& flts)
{
	std::vector<math::Vec3> vec;
	for (unsigned int i = 0; i < flts.size();)
	{
		vec.push_back({ flts[i++], flts[i++], flts[i++] });
	}

	return vec;
}

std::vector<math::Vec2<float>> Object::floats_to_vec2(const std::vector<float>& flts)
{
	std::vector<math::Vec2<float>> vec;
	for (unsigned int i = 0; i < flts.size();)
	{
		vec.push_back({ flts[i++], flts[i++] });
	}

	return vec;
}
