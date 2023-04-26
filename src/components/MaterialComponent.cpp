#include "pch.h"
#include "MaterialComponent.h"
#include "Material.h"
#include "Shader.h"
#include "Texture.h"
#include "ImGuiHelper.h"

#include <imgui.h>
#include <nlohmann/json.hpp>

using namespace nlohmann;

MaterialComponent::MaterialComponent(Material&& material)
{
    m_material = std::make_shared<Material>(std::move(material));
}

MaterialComponent::MaterialComponent(std::shared_ptr<Material> material_ptr)
{
    m_material = material_ptr;
}

void MaterialComponent::set_texturing_mode(TexturingMode mode)
{
    m_texturing_mode = mode;
    m_material->m_using_textures = (mode == TexturingMode::CUSTOM_TEXTURES || mode == TexturingMode::MODEL_DEFAULT);
}

void MaterialComponent::imgui_render()
{
    static std::string combo_preview = ShaderTable::find(m_material->m_shader);
    if (ImGui::BeginCombo("Shader", combo_preview.c_str()))
    {
        for (auto name : ShaderTable::get_material_shaders())
        {
            const bool is_selected = (name == combo_preview);
            if (ImGui::Selectable(name.c_str(), is_selected))
            {
                combo_preview = name;
                m_material->m_shader = ShaderTable::get(combo_preview);
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::Checkbox("Textured", &m_material->m_using_textures);

    // TODO: figure out what it is that you do
    if(!m_material->m_using_textures)
    {
        m_texturing_mode = TexturingMode::NO_TEXTURE;
    }
    else
    {
        m_texturing_mode = TexturingMode::CUSTOM_TEXTURES;
    }

    if (!m_material->m_using_textures)
    {
        float colour[4] = {
                m_material->m_colour.x,
                m_material->m_colour.y,
                m_material->m_colour.z,
                m_material->m_colour.w
        };

        ImGui::ColorEdit4("Base Colour", colour);

        m_material->m_colour.x = colour[0];
        m_material->m_colour.y = colour[1];
        m_material->m_colour.z = colour[2];
        m_material->m_colour.w = colour[3];

        ImGui::SliderFloat("Metallic", &m_material->m_metallic, 0.f, 1.f);
        ImGui::SliderFloat("Roughness", &m_material->m_roughness, 0.f, 1.f);
    }
    else
    {
        ImGui::Text("\nBase Colour\n");
        texture_viewer(m_material->m_textures[0]->get_id(), m_material->m_textures[0]->get_width(), m_material->m_textures[0]->get_height());

        ImGui::Text("\nMetallic Roughness\n");
        if(m_material->m_textures[1])
            texture_viewer(m_material->m_textures[1]->get_id(), m_material->m_textures[1]->get_width(), m_material->m_textures[1]->get_height());
        else
            display_empty_texture();

        ImGui::Text("\nNormal Map\n");
        if(m_material->m_textures[2])
            texture_viewer(m_material->m_textures[2]->get_id(), m_material->m_textures[2]->get_width(), m_material->m_textures[2]->get_height());
        else
            display_empty_texture();

        ImGui::Text("\nOcclusion Map\n");
        if(m_material->m_textures[3])
            texture_viewer(m_material->m_textures[3]->get_id(), m_material->m_textures[3]->get_width(), m_material->m_textures[3]->get_height());
        else
            display_empty_texture();
    }
}

void MaterialComponent::serialize(json& accessor) const
{
    accessor["material"]["texturing_mode"] = (int)m_texturing_mode;

    accessor["material"]["properties"]["colour"][0] = m_material->m_colour.x;
    accessor["material"]["properties"]["colour"][1] = m_material->m_colour.y;
    accessor["material"]["properties"]["colour"][2] = m_material->m_colour.z;
    accessor["material"]["properties"]["colour"][3] = m_material->m_colour.w;
    accessor["material"]["properties"]["metallic_property"] = m_material->m_metallic;
    accessor["material"]["properties"]["roughness"] = m_material->m_roughness;

    accessor["material"]["textures"]["base_colour"] = m_material->m_texture_locations[0];
    accessor["material"]["textures"]["specular"]    = m_material->m_texture_locations[1];
    accessor["material"]["textures"]["normal_map"]  = m_material->m_texture_locations[2];
    accessor["material"]["textures"]["occlusion"]   = m_material->m_texture_locations[3];

    accessor["material"]["shader"] = ShaderTable::find(m_material->m_shader);
}

