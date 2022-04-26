#include "pch.h"
#include "Camera.h"

#include "Input.h"

#define DEG_TO_RAD(x) (x / 180.f) * 3.1415f

Camera::Camera()
{
	m_position = { 0.f, 0.f, 0.f };

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

math::Mat4 Camera::look_at_no_translate()
{
	math::Vec3 f(m_position - m_forward);
	f.normalize();
	math::Vec3 s(f.cross(m_up));
	s.normalize();
	math::Vec3 u(s.cross(f));

	math::Mat4 Result;
	Result(0,0) = s.x;
	Result(1,0) = s.y;
	Result(2,0) = s.z;
	Result(0,1) = u.x;
	Result(1,1) = u.y;
	Result(2,1) = u.z;

	return Result;
}

void Camera::update(float elapsed_time)
{
	if (Input::is_key_pressed(GLFW_KEY_W))
	{
		move_forward(0.001f * elapsed_time);
	}

	if (Input::is_key_pressed(GLFW_KEY_S))
	{
		move_forward(-0.001f * elapsed_time);
	}

	if (Input::is_key_pressed(GLFW_KEY_A))
	{
		move_right(-0.001f * elapsed_time);
	}

	if (Input::is_key_pressed(GLFW_KEY_D))
	{
		move_right(0.001f * elapsed_time);
	}

	if (Input::is_key_pressed(GLFW_KEY_R))
	{
		reset();
	}

	if (Input::is_button_pressed(GLFW_MOUSE_BUTTON_2))
	{
		if (!m_mouse_down)
		{
			Input::set_mouse_pos((1280 / 2), (960 / 2));

			m_mouse_down = true;
		}

		auto [x, y] = Input::get_mouse_pos();

		float rotX = elapsed_time * m_sensitivity * (float)(y - (960 / 2)) / 1280;
		float rotY = elapsed_time * m_sensitivity * (float)(x - (1280 / 2)) / 960;

		math::Quaternion qx(-DEG_TO_RAD(rotX), m_right);
		math::Quaternion qy(-DEG_TO_RAD(rotY), m_up);

		rotate(qx * qy);

		Input::set_mouse_pos((1280 / 2), (960 / 2));
	}
	else
	{
		m_mouse_down = false;
	}

}

void Camera::rotate(math::Quaternion q)
{
	math::Mat4 rotation = q.convert_to_mat();

	math::Vec3 new_forward = rotation * m_forward;
	m_forward.normalize();

	float diff = new_forward.dot(m_up);

	if(diff < 0.05f)
	{
		m_forward = new_forward;

		m_right = rotation * m_right;
		m_right.normalize();

		m_up = rotation * m_up;
		m_up.normalize();
	}

	/*m_up = m_forward.cross(m_right);
	m_up.normalize();*/
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
