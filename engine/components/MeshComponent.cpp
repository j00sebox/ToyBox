#include "pch.h"
#include "MeshComponent.h"
// #include "Mesh.h"

#include <imgui.h>
#include <nlohmann/json.hpp>

using namespace nlohmann;

void MeshComponent::bind() const
{
   // m_mesh->bind();
}

void MeshComponent::unbind() const
{
   // m_mesh->unbind();
}

void MeshComponent::imgui_render()
{
    ImGui::Text("name: ");
    ImGui::SameLine();
    ImGui::TextUnformatted(m_mesh_name.c_str());

    ImGui::Text("\nStenciling: ");
    ImGui::SliderFloat("Outlining Factor", &m_outlining_factor, 0.f, 0.3f);
    ImGui::Checkbox("Use Scale Outline", &m_use_scale_outline);
}

void MeshComponent::serialize(json& accessor) const
{
	accessor["mesh"]["mesh_name"] = m_mesh_name;
    accessor["mesh"]["mesh_type"] = m_mesh_type;
    //accessor["mesh"]["instanced"] = m_mesh->is_instanced();

    accessor["mesh"]["outlining_factor"] = m_outlining_factor;
    accessor["mesh"]["use_scale_outline"] = m_use_scale_outline;
}

