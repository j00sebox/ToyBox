#include "pch.h"
#include "Mesh.h"

#include "GLError.h"

#include "Buffer.h"

#include <mathz/Matrix.h>

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

void Mesh::load(const std::vector<float>& verts, const std::vector<unsigned int>& indices)
{
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

void Mesh::load_primitive(PrimitiveTypes primitive)
{
	switch (primitive)
	{
	case PrimitiveTypes::Cube:
		Cube cube;
		m_va = cube.get_va();
		m_indices_count = cube.get_index_count();
		break;
	}
}

void Mesh::bind() const
{
	m_va.bind();
}

void Mesh::unbind() const
{
	m_va.unbind();
}

void Mesh::imgui_render()
{
	ImGui::Text(m_path.c_str());
}

