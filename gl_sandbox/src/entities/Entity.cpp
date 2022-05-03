#include "pch.h"
#include "Entity.h"

#include "Shader.h"
#include "GLError.h"

#include "mathz/Quaternion.h"
#include "mathz/Misc.h"

void Entity::set_name(const std::string& name)
{
	m_name = name;
}

void Entity::set_shader(const std::string& shader_name)
{
	m_shader_name = shader_name;
}