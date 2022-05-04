#include "pch.h"
#include "Mesh.h"

#include "GLError.h"
#include "GLTFLoader.h"
#include "mathz/Matrix.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <glad/glad.h>

Mesh::Mesh(Mesh&& mesh) noexcept
{
	m_va = std::move(mesh.m_va);
	m_indices_count = mesh.m_indices_count;
	m_textures = std::move(mesh.m_textures);
	m_path = std::move(mesh.m_path);
}

void Mesh::draw() const
{
	m_va.bind();
	for (unsigned int i = 0; i < m_textures.size(); ++i)
	{
		m_textures[i].bind(i);
	}
	GL_CALL(glDrawElements(GL_TRIANGLES, m_indices_count, GL_UNSIGNED_INT, nullptr));
}

void Mesh::load(const std::string& file_path)
{
	GLTFLoader loader(file_path.c_str());

	std::vector<mathz::Vec3> positions = floats_to_vec3(loader.get_positions());
	std::vector<mathz::Vec3> normals = floats_to_vec3(loader.get_normals());
	std::vector<mathz::Vec2<float>> tex_coords = floats_to_vec2(loader.get_tex_coords());

	m_textures.emplace_back(Texture2D(loader.get_base_color_texture()));
	//m_textures.emplace_back(Texture2D(loader.get_specular_color_texture()));

	std::vector<Vertex> vertices;

	for (unsigned int i = 0; i < positions.size(); i++)
	{
		vertices.push_back({
				positions[i],
				normals[i],
				tex_coords[i]
			});
	}

	std::vector<float> verts;

	for (const Vertex& v : vertices)
	{
		verts.push_back(v.positon.x);
		verts.push_back(v.positon.y);
		verts.push_back(v.positon.z);
		verts.push_back(v.normal.x);
		verts.push_back(v.normal.y);
		verts.push_back(v.normal.z);
		verts.push_back(v.st.x);
		verts.push_back(v.st.y);
	}

	std::vector<unsigned int> indices = loader.get_indices();

	m_indices_count = indices.size();

	m_va.bind();

	VertexBuffer vb(verts);
	IndexBuffer ib(indices);

	BufferLayout layout = {
		{0, 3, GL_FLOAT, false},
		{1, 3, GL_FLOAT, false},
		{2, 2, GL_FLOAT, false}
	};

	m_va.set_layout(vb, layout);

	m_va.unbind();
	ib.unbind();
	vb.unbind();
}

void Mesh::parse(json mesh)
{
	m_path = mesh["path"];
	load(m_path);
}

void Mesh::imgui_render()
{
	ImGui::Text("\nMesh\n");
	ImGui::Text(m_path.c_str());
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
