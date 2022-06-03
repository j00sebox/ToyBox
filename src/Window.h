#pragma once

#include "renderer/Renderer.h"

#define GLFW_INCLUDE_NONE

class GLFWwindow;
class ViewPort;

class Window
{
public:
	Window(int width, int height, int viewport_width, int viewport_height);
	~Window();

	void display_render_context();
    void resize_frame_buffer(int width, int height);

	void begin_frame();
	void end_frame();
	[[nodiscard]] float get_delta_time();
    [[nodiscard]] std::pair<int, int> get_dimensions() { return { m_width, m_height }; }

private:
	int m_width, m_height;
	double prev_time = 0.0;
	GLFWwindow* m_window_handle;
    std::unique_ptr<ViewPort> m_main_viewport;
};

