#pragma once
#include "Types.hpp"

#include <GLFW/glfw3.h>

class Input
{
public:
	static bool is_key_pressed(i32 key_code);
	static bool is_button_pressed(i32 button_code);
	static void set_mouse_pos(u32 x, u32 y);
	static std::pair<f32, f32> get_mouse_pos();
	static void show_cursor(bool show);

	static GLFWwindow* m_window_handle;
};