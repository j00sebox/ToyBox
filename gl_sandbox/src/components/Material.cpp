#include "pch.h"
#include "Material.h"

#include <imgui.h>
#include <imgui_internal.h>

void Material::load(const GLTFLoader& loader)
{
	std::vector<std::string> strs = loader.get_textures();

	for (auto& s : strs)
	{
		if (!s.empty())
		{
			m_textures.emplace_back(Texture2D(s));
		}
	}
}

void Material::bind() const
{
	for (unsigned int i = 0; i < m_textures.size(); ++i)
	{
		m_textures[i].bind(i);
	}
	m_shader->bind();
}

void Material::imgui_render()
{
	ImGui::Text("\nMaterial\n");
}
