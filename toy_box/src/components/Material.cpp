#include "pch.h"
#include "Material.h"

#include <imgui.h>
#include <imgui_internal.h>

void Material::load(const std::string* const textures)
{
    m_textures[0] = std::make_unique<Texture2D>(Texture2D(textures[0]));
    m_textures[1] = std::make_unique<Texture2D>(Texture2D(textures[1]));
    m_textures[2] = std::make_unique<Texture2D>(Texture2D(textures[2]));
    m_textures[3] = std::make_unique<Texture2D>(Texture2D(textures[3]));

    m_custom = false;
}

void Material::bind() const
{
    m_shader->set_uniform_1i("u_custom", m_custom);

    if (m_custom)
    {
        m_shader->set_uniform_4f("u_flat_colour", m_colour);
        m_shader->set_uniform_1f("u_metallic", m_metallic);
        m_shader->set_uniform_1f("u_roughness", m_roughness);
    }
    else
    {
        for (unsigned int i = 0; i < 4; ++i)
        {
            m_textures[i]->bind(i);
        }
    }
    
	m_shader->bind();
}

void Material::unbind() const
{

    if (!m_custom)
    {
        for (unsigned int i = 0; i < 4; ++i)
        {
            m_textures[i]->unbind();
        }
    }
    
    m_shader->unbind();
}

void Material::texture_viewer(unsigned int texture_index)
{
    ImGuiIO& io = ImGui::GetIO();
    ImTextureID my_tex_id = (ImTextureID)m_textures[texture_index]->get_id();
    float my_tex_w = (float)m_textures[texture_index]->get_width();
    float my_tex_h = (float)m_textures[texture_index]->get_height();
    {
        ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 uv_min = ImVec2(0.0f, 0.0f);
        ImVec2 uv_max = ImVec2(1.0f, 1.0f);
        ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
        ImGui::Image(my_tex_id, ImVec2(my_tex_w * 0.05f, my_tex_h * 0.05f), uv_min, uv_max, tint_col, border_col);
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            float region_sz = 32.0f;
            float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
            float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
            float zoom = 4.0f;
            if (region_x < 0.0f) { region_x = 0.0f; }
            else if (region_x > my_tex_w - region_sz) { region_x = my_tex_w - region_sz; }
            if (region_y < 0.0f) { region_y = 0.0f; }
            else if (region_y > my_tex_h - region_sz) { region_y = my_tex_h - region_sz; }
            ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
            ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
            ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
            ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
            ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, tint_col, border_col);
            ImGui::EndTooltip();
        }
    }
}

void Material::imgui_render()
{
    ImGui::Checkbox("Custom", &m_custom);

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
        texture_viewer(0);

        ImGui::Text("\nMetallic Roughness\n");
        texture_viewer(1);

        ImGui::Text("\nNormal Map\n");
        texture_viewer(2);

        ImGui::Text("\nOcclusion Map\n");
        texture_viewer(3);
    }

    
}
