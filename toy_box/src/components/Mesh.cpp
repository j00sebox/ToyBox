#include "pch.h"
#include "Mesh.h"

#include "GLError.h"
#include "Buffer.h"
#include "VertexArray.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <mathz/Matrix.h>
#include <glad/glad.h>
#include <json/json.hpp>

using namespace nlohmann;

Mesh::Mesh(Mesh&& mesh) noexcept
{
	m_va = std::move(mesh.m_va);
	m_indices_count = mesh.m_indices_count;
	
	// TODO: Remove later
	m_gltf_path = mesh.m_gltf_path;
	m_primitive = mesh.m_primitive;
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
	// TODO: Figure out better way to do this
	m_primitive = primitive;

	switch (primitive)
	{
		case PrimitiveTypes::Cube:
		{
			Cube cube;
			m_va = cube.get_va();
			m_indices_count = cube.get_index_count();
			break;
		}
		case PrimitiveTypes::Plane:
		{
			Plane plane;
			m_va = plane.get_va();
			m_indices_count = plane.get_index_count();
			break;
		}
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
	
}

void Mesh::serialize(json& accessor) const
{
	if (!m_gltf_path.empty())
	{
		accessor["gltf"]["path"] = m_gltf_path;
	}
	else if (m_primitive != PrimitiveTypes::None)
	{
		accessor["primitive"] = primitve_type_to_str(m_primitive);
	}
}

