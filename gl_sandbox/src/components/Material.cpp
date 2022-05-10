#include "pch.h"
#include "Material.h"

#include <imgui.h>
#include <imgui_internal.h>

void Material::load(const GLTFLoader& loader)
{
    m_textures[0] = std::make_unique<Texture2D>(Texture2D(loader.get_base_color_texture()));
    m_textures[1] = std::make_unique<Texture2D>(Texture2D(loader.get_specular_texture()));
    m_textures[2] = std::make_unique<Texture2D>(Texture2D(loader.get_normal_texture()));
    m_textures[3] = std::make_unique<Texture2D>(Texture2D(loader.get_occlusion_texture()));
}

void Material::bind() const
{
    if (m_textures[0])
    {
        for (unsigned int i = 0; i < 4; ++i)
        {
            m_textures[i]->bind(i);
        }
    }
	m_shader->bind();
}

void Material::imgui_render()
{
    if (m_textures[0] == nullptr)
        return;

    for(unsigned int i = 0; i < 4; ++i)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImTextureID my_tex_id = (ImTextureID)m_textures[i]->get_id();
        float my_tex_w = (float)m_textures[i]->get_width();
        float my_tex_h = (float)m_textures[i]->get_height();
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
}
