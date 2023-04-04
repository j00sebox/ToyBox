#include "pch.h"
#include "MeshObject.h"
#include "Mesh.h"

#include <imgui.h>
#include <nlohmann/json.hpp>

using namespace nlohmann;

void MeshObject::bind() const
{
    m_mesh->bind();
}

void MeshObject::unbind() const
{
    m_mesh->unbind();
}

void MeshObject::imgui_render()
{
    ImGui::Text("name: ");
    ImGui::SameLine();
    ImGui::Text(m_mesh_name.c_str());
}

void MeshObject::serialize(json& accessor) const
{
	accessor["mesh"]["mesh_name"] = m_mesh_name;
    accessor["mesh"]["mesh_type"] = m_mesh_type;
    accessor["mesh"]["instanced"] = m_mesh->is_instanced();
}

