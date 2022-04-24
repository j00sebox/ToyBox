#include "pch.h"
#include "Window.h"

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

		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			EventList::e_camera_move.execute_function(0.f, 0.f, -0.25f);
		}
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

	m_renderer.reset(new Renderer(width, height));

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
		double time = glfwGetTime() * 1000.0;

		m_renderer->update((float)(time - prev_time));

		prev_time = time;
		
		glfwSwapBuffers(m_window_handle);
		glfwPollEvents();
	}
}
