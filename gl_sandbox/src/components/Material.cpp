#include "pch.h"
#include "Material.h"

#include <imgui.h>
#include <imgui_internal.h>

void Material::bind() const
{
	for (unsigned int i = 0; i < m_textures.size(); ++i)
	{
		m_textures[i].bind(i);
	}
	m_shader->bind();
}

void Material::load(const GLTFLoader& loader)
{
	m_textures.emplace_back(Texture2D(loader.get_base_color_texture()));
	//m_textures.emplace_back(Texture2D(loader.get_specular_color_texture()));
}

void Material::imgui_render()
{
	ImGui::Text("\nMaterial\n");
}
