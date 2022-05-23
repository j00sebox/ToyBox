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

	void begin_frame();
	void end_frame();
	float get_delta_time();
	std::pair<int, int> get_dimensions() { return std::pair<int, int>(m_width, m_height); }

private:
	int m_width, m_height;
	double prev_time = 0.0;
	GLFWwindow* m_window_handle;
	std::unique_ptr<FrameBuffer> m_frame_buffer;
};

