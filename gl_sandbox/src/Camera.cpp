#include "pch.h"
#include "Camera.h"

Camera::Camera()
{
	m_forward = { 0.f, 0.f, 1.f };
	m_up = { 0.f, 1.f, 0.f };
	m_right = { 1.f, 0.f, 0.f };
}

math::Mat4 Camera::camera_look_at()
{
	// Set up camera direction matrix
	m_transform(0, 0) = m_right.x;			m_transform(0, 1) = m_right.y;			m_transform(0, 2) = m_right.z;			m_transform(0, 3) = 0.0f;
	m_transform(1, 0) = m_up.x;				m_transform(1, 1) = m_up.y;				m_transform(1, 2) = m_up.z;				m_transform(1, 3) = 0.0f;
	m_transform(2, 0) = m_forward.x;		m_transform(2, 1) = m_forward.y;		m_transform(2, 2) = m_forward.z;		m_transform(2, 3) = 0.0f;
	m_transform(3, 0) = m_position.x;		m_transform(3, 1) = m_position.y;		m_transform(3, 2) = m_position.z;		m_transform(3, 3) = 1.0f;

	return m_transform.inverse();
}

void Camera::rotate(math::Quaternion q)
{
	math::Mat4 rotation = q.convert_to_mat();

	m_forward = rotation * m_forward;
	m_forward.normalize();

	m_right = rotation * m_right;
	m_right.normalize();

	m_up = m_forward.cross(m_right);
	m_up.normalize();
}

void Camera::move_forward(float f)
{
	m_position = m_position + (m_forward * f);
}

void Camera::move_right(float r)
{
	m_position = m_position + (m_right * r);
}

void Camera::set_pos(math::Vec3&& pos)
{
    m_position = pos;
}

const math::Vec3& Camera::get_pos() const
{
	return m_position;
}

void Camera::reset()
{
	m_forward = { 0.f, 0.f, 1.f };
	m_up = { 0.f, 1.f, 0.f };
	m_right = { 1.f, 0.f, 0.f };

	m_position = { 0.f, 0.f, 0.f };
}
