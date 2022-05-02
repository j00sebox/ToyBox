#include "pch.h"
#include "SceneObject.h"

#include "Shader.h"
#include "GLError.h"

#include "mathz/Quaternion.h"
#include "mathz/Misc.h"

void SceneObject::translate(const mathz::Vec3& pos)
{
	m_postion = pos;
	m_translate[3][0] = pos.x;	m_translate[3][1] = pos.y;	m_translate[3][2] = pos.z;
}

void SceneObject::scale(float s)
{
	m_scale[0][0] = s; m_scale[1][1] = s; m_scale[2][2] = s;
}

void SceneObject::rotate(float angle, const mathz::Vec3& axis)
{
	m_rotate_angle = angle;
	m_rotate_axis = axis;
	mathz::Quaternion q(mathz::radians(angle), axis);
	m_rotation = q.convert_to_mat();
}

void SceneObject::set_name(const std::string& name)
{
	m_name = name;
}

void SceneObject::set_shader(const std::string& shader_name)
{
	m_shader_name = shader_name;
}

mathz::Mat4 SceneObject::get_transform() const
{
	return m_translate * m_rotation * m_scale;
}
