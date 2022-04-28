#include "pch.h"
#include "Mesh.h"

#include "GLError.h"
#include "GLTFLoader.h"
#include "mathz/Matrix.h"

#include <glad/glad.h>

Mesh::Mesh(const std::string& file_path)
{
	GLTFLoader loader(file_path.c_str());

	std::vector<mathz::Vec3> positions = floats_to_vec3(loader.get_positions());
	std::vector<mathz::Vec2<float>> tex_coords = floats_to_vec2(loader.get_tex_coords());

	m_texture.reset(new Texture2D(loader.get_base_color_texture()));

	for (unsigned int i = 0; i < positions.size(); i++)
	{
		m_vertices.push_back({
				positions[i],
				tex_coords[i]
			});
	}

	std::vector<float> vertices = get_vertices();
	std::vector<unsigned int> indices = loader.get_indices();

	VertexBuffer vb = VertexBuffer();
	vb.add_data(vertices.data(), vertices.size() * sizeof(float));

	m_ib.reset(new IndexBuffer(indices.data(), indices.size() * sizeof(unsigned int)));

	BufferLayout layout = {
		{0, 3, GL_FLOAT, false},
		{1, 2, GL_FLOAT, false}
	};

	m_va.set_layout(vb, layout);

	m_va.unbind();
	m_ib->unbind();
	vb.unbind();
}

void Mesh::draw() const
{
	m_va.bind();
	m_ib->bind();
	m_texture->bind(0);
	GL_CALL(glDrawElements(GL_TRIANGLES, m_ib->get_count(), GL_UNSIGNED_INT, nullptr));
}

std::vector<float> Mesh::get_vertices() const
{
	std::vector<float> verts;

	for (unsigned int i = 0; i < m_vertices.size(); ++i)
	{
		verts.push_back(m_vertices[i].positon.x);
		verts.push_back(m_vertices[i].positon.y);
		verts.push_back(m_vertices[i].positon.z);
		verts.push_back(m_vertices[i].st.x);
		verts.push_back(m_vertices[i].st.y);
	}

	return verts;
}

std::vector<mathz::Vec3> Mesh::floats_to_vec3(const std::vector<float>& flts) const
{
	std::vector<mathz::Vec3> vec;
	for (unsigned int i = 0; i < flts.size();)
	{
		vec.push_back({ flts[i++], flts[i++], flts[i++] });
	}

	return vec;
}

std::vector<mathz::Vec2<float>> Mesh::floats_to_vec2(const std::vector<float>& flts) const
{
	std::vector<mathz::Vec2<float>> vec;
	for (unsigned int i = 0; i < flts.size();)
	{
		vec.push_back({ flts[i++], flts[i++] });
	}

	return vec;
}
