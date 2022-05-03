#include "pch.h"
#include "Transform.h"

#include "mathz/Quaternion.h"
#include "mathz/Misc.h"

void Transform::translate(const mathz::Vec3& pos)
{
	m_postion = pos;
	m_translate[3][0] = pos.x;	m_translate[3][1] = pos.y;	m_translate[3][2] = pos.z;
}

void Transform::scale(float s)
{
	m_scale[0][0] = s; m_scale[1][1] = s; m_scale[2][2] = s;
}

void Transform::rotate(float angle, const mathz::Vec3& axis)
{
	m_rotate_angle = angle;
	m_rotate_axis = axis;
	mathz::Quaternion q(mathz::radians(angle), axis);
	m_rotation = q.convert_to_mat();
}

mathz::Mat4 Transform::get_transform() const
{
	return m_translate * m_rotation * m_scale;
}
