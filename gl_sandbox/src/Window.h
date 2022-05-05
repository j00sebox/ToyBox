#pragma once

#include "Renderer.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

class Window
{
public:
	Window(int width, int height);
	~Window();

	void update();
	void begin_frame();
	void end_frame();
	float get_delta_time();

private:
	int m_width, m_height;
	double prev_time = 0.0;
	GLFWwindow* m_window_handle;
	std::unique_ptr<Renderer> m_renderer;
};

