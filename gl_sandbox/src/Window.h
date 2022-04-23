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

	void main_loop();

private:
	int m_width, m_height;
	GLFWwindow* m_window_handle;
	std::unique_ptr<Renderer> m_renderer;
};

