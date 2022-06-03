#pragma once

#include "renderer/Renderer.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

class FrameBuffer;

class Window
{
public:
	Window(int width, int height);
	~Window();

	void display_render_context();
    void resize_frame_buffer(int width, int height);

	void begin_frame();
	void end_frame();
	[[nodiscard]] float get_delta_time();
    [[nodiscard]] std::pair<int, int> get_dimensions() { return { m_width, m_height }; }

private:
	int m_width, m_height;
	float prev_fb_width = 0.f, prev_fb_height = 0.f;
	double prev_time = 0.0;
	GLFWwindow* m_window_handle;
	std::unique_ptr<FrameBuffer> m_frame_buffer;
    std::unique_ptr<FrameBuffer> m_multisample_frame_buffer;
};

