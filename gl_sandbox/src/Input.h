#pragma once

#include <GLFW/glfw3.h>

class Input
{
public:
	static bool is_key_pressed(GLFWwindow* window_handle, int key_code);
	static bool is_button_pressed(GLFWwindow* window_handle, int button_code);
};