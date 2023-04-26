#include "pch.h"
#include "Material.h"
#include "Texture.h"
#include "Shader.h"
#include "Log.h"

void Material::load(const std::string* const textures)
{
    m_texture_locations[0] = textures[0];
    m_texture_locations[1] = textures[1];
    m_texture_locations[2] = textures[2];
    m_texture_locations[3] = textures[3];

    m_textures[0] = std::make_unique<Texture2D>(Texture2D(textures[0]));
    m_textures[1] = (textures[1] != "none" && textures[1] != "") ? std::make_unique<Texture2D>(Texture2D(textures[1])) : nullptr;
    m_textures[2] = (textures[2] != "none" && textures[2] != "") ? std::make_unique<Texture2D>(Texture2D(textures[2])) : nullptr;
    m_textures[3] = (textures[3] != "none" && textures[3] != "") ? std::make_unique<Texture2D>(Texture2D(textures[3])) : nullptr;
}

void Material::bind() const
{
    m_shader->set_uniform_1i("u_using_textures", m_using_textures);

    if (!m_using_textures)
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
    if (m_using_textures)
    {
        for (const auto& m_texture : m_textures)
        {
            m_texture->unbind();
        }
    }
    
    m_shader->unbind();
}

std::unordered_map<std::string, std::shared_ptr<Material>> MaterialTable::m_materials;

void MaterialTable::add(const std::string& name, Material&& m)
{
    if (!exists(name))
        m_materials[name] = std::make_shared<Material>(std::move(m));

}

std::shared_ptr<Material> MaterialTable::get(const std::string& name)
{
    if (exists(name))
    {
        return m_materials[name];
    }

    fatal("Material {} does not exist in library!\n", name);
    return nullptr;
}

bool MaterialTable::exists(const std::string &name)
{
    return (m_materials.find(name) != m_materials.end());
}

std::string MaterialTable::find(const std::shared_ptr<Material>& m)
{
    for (const auto& [name, material_ptr] : m_materials)
    {
        if (material_ptr == m)
        {
            return name;
        }
    }

    return "";
}

void MaterialTable::release()
{
    m_materials.clear();
}



