#pragma once

#include <GLFW/glfw3.h>

class Input
{
public:
	static bool is_key_pressed(int key_code);
	static bool is_button_pressed(int button_code);
	static void set_mouse_pos(int x, int y);
	static std::pair<float, float> get_mouse_pos();

	static GLFWwindow* m_window_handle;
};