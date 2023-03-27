#include "pch.h"
#include "Camera.h"

#include "Input.h"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <imgui.h>

Camera::Camera()
	: m_screen_width(0), m_screen_height(0)
{
	reset();
}

glm::mat4 Camera::camera_look_at()
{
	return glm::lookAt(m_position, m_position + m_forward, m_up);
}

glm::mat4 Camera::look_at_no_translate() const
{
    glm::mat4 result;
	result[0][0] = m_right.x;					result[0][1] = m_up.x;					result[0][2] = -m_forward.x;
	result[1][0] = m_right.y;					result[1][1] = m_up.y;					result[1][2] = -m_forward.y;
	result[2][0] = m_right.z;					result[2][1] = m_up.z;					result[2][2] = -m_forward.z;

	return result;
}

void Camera::resize(int width, int height)
{
	m_screen_width = width; m_screen_height = height;

	float scaling_factor = 1.0f / tanf(glm::radians(m_fov) * 0.5f);
	float aspect_ratio = (float)m_screen_height / (float)m_screen_width;

	float q = -1.f / (m_far - m_near);

	// create projection matrices
    m_perspective = glm::mat4(
		aspect_ratio * scaling_factor,	0.f,			0.f,									0.f,
		0.f,							scaling_factor, 0.f,									0.f,
		0.f,							0.f,			(m_far + m_near) * q,				   -1.f,
		0.f,							0.f,			2.f * m_near * m_far * q,				0.f
	);

	m_orthographic = glm::mat4(
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

			// calculate vertical orientation adjustment
			glm::vec3 new_forward = glm::rotate(m_forward, glm::radians(-rotX), m_right);

			// prevents barrel rolls
			if (abs(glm::angle(new_forward, m_up) - glm::radians(90.0f)) <= glm::radians(85.0f))
			{
				m_forward = new_forward;
			}

			// get horizontal orientation adjustment and new right vector
			m_forward = glm::rotate(m_forward, glm::radians(-rotY), m_up);
			m_right = glm::normalize(glm::cross(m_forward, m_up));

			// keep mouse in the center
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

void Camera::move_forward(float f)
{
	m_position = m_position + (m_forward * f);
}

void Camera::move_right(float r)
{
	m_position = m_position + (m_right * r);
}

void Camera::set_pos(glm::vec3&& pos)
{
    m_position = pos;
}

const glm::vec3& Camera::get_pos() const
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
