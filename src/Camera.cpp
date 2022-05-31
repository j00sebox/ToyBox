#include "pch.h"
#include "Camera.h"

#include "Input.h"

#include <mathz/Misc.h>

#include <imgui.h>

Camera::Camera()
	: m_screen_width(0), m_screen_height(0)
{
	m_position = { 0.f, 0.f, 0.f };

	m_forward = { 0.f, 0.f, -1.f };
	m_up = { 0.f, 1.f, 0.f };
	m_right = { 1.f, 0.f, 0.f };
}

mathz::Mat4 Camera::camera_look_at()
{
	mathz::Mat4 result;
	result[0][0] = m_right.x;					result[0][1] = m_up.x;					result[0][2] = -m_forward.x;
	result[1][0] = m_right.y;					result[1][1] = m_up.y;					result[1][2] = -m_forward.y;
	result[2][0] = m_right.z;					result[2][1] = m_up.z;					result[2][2] = -m_forward.z;
	result[3][0] = -m_right.dot(m_position);	result[3][1] = -m_up.dot(m_position);	result[3][2] = m_forward.dot(m_position);

	return result;
}

mathz::Mat4 Camera::look_at_no_translate() const
{
	mathz::Mat4 result;
	result[0][0] = m_right.x;					result[0][1] = m_up.x;					result[0][2] = -m_forward.x;
	result[1][0] = m_right.y;					result[1][1] = m_up.y;					result[1][2] = -m_forward.y;
	result[2][0] = m_right.z;					result[2][1] = m_up.z;					result[2][2] = -m_forward.z;

	return result;
}

void Camera::resize(int width, int height)
{
	m_screen_width = width; m_screen_height = height;

	float scaling_factor = 1.0f / tanf(mathz::radians(m_fov) * 0.5f);
	float aspect_ratio = (float)m_screen_height / (float)m_screen_width;

	float q = -1.f / (m_far - m_near);

	// create projection matrices
	m_perspective.set(
		aspect_ratio * scaling_factor,	0.f,			0.f,									0.f,
		0.f,							scaling_factor, 0.f,									0.f,
		0.f,							0.f,			(m_far + m_near) * q,				   -1.f,
		0.f,							0.f,			2.f * m_near * m_far * q,				0.f
	);

	m_orthographic.set(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f / (m_far - 0.1f), -m_near / (m_far - m_near),
		0.f, 0.f, 0.f, 1.f
	);
}

bool Camera::update(float elapsed_time)
{
	// block camera update if imgui menu is in use
	if (!ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive())
	{
		if (Input::is_key_pressed(GLFW_KEY_W))
		{
			move_forward(m_speed * elapsed_time);
		}

		if (Input::is_key_pressed(GLFW_KEY_S))
		{
			move_forward(-m_speed * elapsed_time);
		}

		if (Input::is_key_pressed(GLFW_KEY_A))
		{
			move_right(-m_speed * elapsed_time);
		}

		if (Input::is_key_pressed(GLFW_KEY_D))
		{
			move_right(m_speed * elapsed_time);
		}

		if (Input::is_key_pressed(GLFW_KEY_R))
		{
			reset();
		}

		if (Input::is_button_pressed(GLFW_MOUSE_BUTTON_2))
		{
			if (!m_mouse_down)
			{
				Input::show_cursor(false);
				Input::set_mouse_pos((m_screen_width / 2), (m_screen_height / 2));

				m_mouse_down = true;
			}

			auto [x, y] = Input::get_mouse_pos();

			float rotX = elapsed_time * m_sensitivity * (y - (float)(m_screen_height / 2)) / (float)m_screen_width;
			float rotY = elapsed_time * m_sensitivity * (x - (float)(m_screen_width / 2)) / (float)m_screen_height;

			mathz::Quaternion qx(mathz::radians(rotX), m_right);
			mathz::Quaternion qy(mathz::radians(rotY), m_up);

			rotate(qx * qy);

			Input::set_mouse_pos((m_screen_width / 2), (m_screen_height / 2));
		}
		else
		{
			m_mouse_down = false;

			Input::show_cursor(true);
		}

        return true;
	}

    return false;
}

void Camera::rotate(mathz::Quaternion q)
{
	mathz::Mat4 rotation = q.convert_to_mat();

	m_forward = rotation * m_forward;
	m_forward.normalize();

	m_right = rotation * m_right;
	m_right.normalize();

	m_up = m_right.cross(m_forward);
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

void Camera::set_pos(mathz::Vec3&& pos)
{
    m_position = pos;
}

const mathz::Vec3& Camera::get_pos() const
{
	return m_position;
}

void Camera::reset()
{
	m_forward = { 0.f, 0.f, -1.f };
	m_up = { 0.f, 1.f, 0.f };
	m_right = { 1.f, 0.f, 0.f };

	m_position = { 0.f, 0.f, 0.f };
}
