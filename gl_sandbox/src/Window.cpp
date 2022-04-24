#include "pch.h"
#include "Window.h"

#include "Shader.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Texture.h"

#include "GLError.h"

#include "math/Matrix.h"

extern "C"
{
	void glfw_error_callback(int error, const char* description)
	{
		fprintf(stderr, "[%i] Error: %s\n", error, description);
	}

	void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

Window::Window(int width, int height)
	: m_width(width), m_height(height)
{
	if (!glfwInit())
		__debugbreak();

	m_window_handle = glfwCreateWindow(width, height, "gl_sandbox", NULL, NULL);

	if (!m_window_handle)
		__debugbreak();

	glfwMakeContextCurrent(m_window_handle);
	gladLoadGL();

	// v-sync on
	glfwSwapInterval(1);

	// setup callbacks
	glfwSetErrorCallback(glfw_error_callback);
	glfwSetKeyCallback(m_window_handle, glfw_key_callback);

	glViewport(0, 0, width, height);

	m_renderer.reset(new Renderer());

	main_loop();
}

Window::~Window()
{
	glfwDestroyWindow(m_window_handle);
	glfwTerminate();
}

void Window::main_loop()
{
	while (!glfwWindowShouldClose(m_window_handle))
	{
		double time = glfwGetTime();

		m_renderer->update(0.f);
		
		glfwSwapBuffers(m_window_handle);
		glfwPollEvents();
	}
}
