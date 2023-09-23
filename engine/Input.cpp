#include "pch.h"
#include "Input.hpp"

GLFWwindow* Input::m_window_handle = nullptr;

bool Input::is_key_pressed(i32 key_code)
{
	auto state = glfwGetKey(m_window_handle, key_code);
	return (state == GLFW_PRESS) || (state == GLFW_REPEAT);
}

bool Input::is_button_pressed(i32 button_code)
{
	auto state = glfwGetMouseButton(m_window_handle, button_code);
	return (state == GLFW_PRESS) || (state == GLFW_REPEAT);
}

void Input::set_mouse_pos(u32 x, u32 y)
{
	glfwSetCursorPos(m_window_handle, x, y);
}

std::pair<f32, f32> Input::get_mouse_pos()
{
	f64 xPos, yPos;
	glfwGetCursorPos(m_window_handle, &xPos, &yPos);

	return { (f32)xPos, (f32)yPos };
}

void Input::show_cursor(bool show)
{
	if(show)
    {
        glfwSetInputMode(m_window_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
	else
    {
        glfwSetInputMode(m_window_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}
