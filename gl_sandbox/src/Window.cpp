#include "pch.h"
#include "Window.h"

#include "Log.h"
#include "Input.h"
#include "events/EventList.h"

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
		ASSERT(false);

	m_window_handle = glfwCreateWindow(width, height, "gl_sandbox", NULL, NULL);

	if (!m_window_handle)
		ASSERT(false);

	glfwMakeContextCurrent(m_window_handle);
	gladLoadGL();

	// v-sync on
	glfwSwapInterval(1);

	// setup callbacks
	glfwSetErrorCallback(glfw_error_callback);
	glfwSetKeyCallback(m_window_handle, glfw_key_callback);

	m_renderer.reset(new Renderer(width, height));

	Input::m_window_handle = m_window_handle;

	main_loop();
}

Window::~Window()
{
	m_renderer.release();
	glfwDestroyWindow(m_window_handle);
	glfwTerminate();
}

void Window::main_loop()
{
	while (!glfwWindowShouldClose(m_window_handle))
	{
		float dx = 0.f, dy = 0.f;
		double time = glfwGetTime() * 1000.0;

		float delta_time = (float)(time - prev_time);

		m_renderer->update(delta_time);

		prev_time = time;
		
		glfwSwapBuffers(m_window_handle);
		glfwPollEvents();
	}
}
