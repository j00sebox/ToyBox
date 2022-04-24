#include "pch.h"
#include "Input.h"

bool Input::is_key_pressed(GLFWwindow* window_handle, int key_code)
{
	auto state = glfwGetKey(window_handle, key_code);
	return (state == GLFW_PRESS) || (state == GLFW_REPEAT);
}

bool Input::is_button_pressed(GLFWwindow* window_handle, int button_code)
{
	auto state = glfwGetMouseButton(window_handle, button_code);
	return (state == GLFW_PRESS) || (state == GLFW_REPEAT);
}

std::pair<float, float> Input::get_mouse_pos(GLFWwindow* window_handle)
{
	double xPos, yPos;
	glfwGetCursorPos(window_handle, &xPos, &yPos);

	return std::pair<float, float>((float)xPos, (float)yPos);
}