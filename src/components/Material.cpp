#include "pch.h"
#include "Material.h"
#include "Texture.h"
#include "Shader.h"
#include "ImGuiHelper.h"

#include <imgui.h>
#include <nlohmann/json.hpp>

using namespace nlohmann;

void Material::load(const std::string* const textures)
{
    m_textures[0] = std::make_unique<Texture2D>(Texture2D(textures[0]));
    m_textures[1] = (textures[1] != "none") ? std::make_unique<Texture2D>(Texture2D(textures[1])) : nullptr;
    m_textures[2] = (textures[2] != "none") ? std::make_unique<Texture2D>(Texture2D(textures[2])) : nullptr;
    m_textures[3] = (textures[3] != "none") ? std::make_unique<Texture2D>(Texture2D(textures[3])) : nullptr;

    m_custom = false;
}

void Material::bind() const
{
    m_shader->set_uniform_1i("u_custom", m_custom);

    if (m_custom)
    {
        m_shader->set_uniform_4f("u_base_colour", m_colour);
        m_shader->set_uniform_1f("u_metallic", m_metallic);
        m_shader->set_uniform_1f("u_roughness", m_roughness);
    }
    else
    {
        for (unsigned int i = 0; i < 4; ++i)
        {
            if(m_textures[i])
                m_textures[i]->bind(i);
        }
    }

    m_shader->bind();
}

void Material::unbind() const
{
    if (!m_custom)
    {
        for (const auto& m_texture : m_textures)
        {
            m_texture->unbind();
        }
    }
    
    m_shader->unbind();
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
void Material::imgui_render()
{
    static std::string combo_preview = ShaderTable::find(m_shader);
    if (ImGui::BeginCombo("Shader", combo_preview.c_str()))
    {
        for (auto name : ShaderTable::get_material_shaders())
        {
            const bool is_selected = (name == combo_preview);
            if (ImGui::Selectable(name.c_str(), is_selected))
            {
                combo_preview = name;
                m_shader = ShaderTable::get(combo_preview);
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    bool prev_choice = m_custom;
    ImGui::Checkbox("Custom", &m_custom);

    if(prev_choice && prev_choice != m_custom)
    {
        std::string textures[] = {"../resources/textures/white.png", "none", "none", "none"};
        load(textures);
    }

    if (m_custom)
    {
        float colour[4] = {
            m_colour.x,
            m_colour.y,
            m_colour.z,
            m_colour.w
        };

        ImGui::ColorEdit4("Base Colour", colour);

        m_colour.x = colour[0];
        m_colour.y = colour[1];
        m_colour.z = colour[2];
        m_colour.w = colour[3];

        ImGui::SliderFloat("Metallic", &m_metallic, 0.f, 1.f);
        ImGui::SliderFloat("Roughness", &m_roughness, 0.f, 1.f);
    }
    else
    {
        ImGui::Text("\nBase Colour\n");
        texture_viewer(m_textures[0]->get_id(), m_textures[0]->get_width(), m_textures[0]->get_height());

        ImGui::Text("\nMetallic Roughness\n");
        if(m_textures[1])
            texture_viewer(m_textures[1]->get_id(), m_textures[1]->get_width(), m_textures[1]->get_height());
        else
            display_empty_texture();

        ImGui::Text("\nNormal Map\n");
        if(m_textures[2])
            texture_viewer(m_textures[2]->get_id(), m_textures[2]->get_width(), m_textures[2]->get_height());
        else
            display_empty_texture();

        ImGui::Text("\nOcclusion Map\n");
        if(m_textures[3])
            texture_viewer(m_textures[3]->get_id(), m_textures[3]->get_width(), m_textures[3]->get_height());
        else
            display_empty_texture();
    }
}
#pragma clang diagnostic pop

void Material::serialize(json& accessor) const
{
    if(m_custom)
    {
        accessor["custom_texture"]["colour"][0] = m_colour.x;
        accessor["custom_texture"]["colour"][1] = m_colour.y;
        accessor["custom_texture"]["colour"][2] = m_colour.z;
        accessor["custom_texture"]["colour"][3] = m_colour.w;
    }
}


