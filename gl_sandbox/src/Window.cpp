#include "pch.h"
#include "Window.h"

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

		/*if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			EventList::e_camera_move.execute_function(0.f, 0.f, 0.25f);
		}

		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			EventList::e_camera_move.execute_function(0.f, 0.f, -0.25f);
		}*/
	}

	void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
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
	glfwSetMouseButtonCallback(m_window_handle, glfw_mouse_button_callback);

	m_renderer.reset(new Renderer(width, height));

	main_loop();
}

Window::~Window()
{
	m_renderer.release();
	glfwDestroyWindow(m_window_handle);
	glfwTerminate();
}

#define DEG_TO_RAD(x) (x / 180.f) * 3.141f

void Window::main_loop()
{
	auto [prev_x, prev_y] = Input::get_mouse_pos(m_window_handle);
	bool mouse_down = false;

	while (!glfwWindowShouldClose(m_window_handle))
	{
		float dx = 0.f, dy = 0.f;
		double time = glfwGetTime() * 1000.0;

		float delta_time = (float)(time - prev_time);

		if (Input::is_key_pressed(m_window_handle, GLFW_KEY_W))
		{
			m_renderer->move_cam_forward(0.001f * delta_time);
		}

		if (Input::is_key_pressed(m_window_handle, GLFW_KEY_S))
		{
			m_renderer->move_cam_forward(-0.001f * delta_time);
		}

		if (Input::is_key_pressed(m_window_handle, GLFW_KEY_A))
		{
			m_renderer->move_cam_right(-0.001f * delta_time);
		}

		if (Input::is_key_pressed(m_window_handle, GLFW_KEY_D))
		{
			m_renderer->move_cam_right(0.001f * delta_time);
		}

		if (Input::is_key_pressed(m_window_handle, GLFW_KEY_R))
		{
			m_renderer->reset_view();
		}

		if (Input::is_button_pressed(m_window_handle, GLFW_MOUSE_BUTTON_2))
		{
			if (!mouse_down)
			{
				std::pair<float, float> xy = Input::get_mouse_pos(m_window_handle);

				prev_x = xy.first;
				prev_y = xy.second;

				mouse_down = true;
			}

			auto [x, y] = Input::get_mouse_pos(m_window_handle);

			dx = DEG_TO_RAD((x - prev_x) * delta_time * 0.001f);
			dy = DEG_TO_RAD((y - prev_y) * delta_time * 0.001f);

			m_renderer->update_camera_rot(dx, -dy);
		}
		else
			mouse_down = false;

		m_renderer->update(delta_time);

		prev_time = time;
		
		glfwSwapBuffers(m_window_handle);
		glfwPollEvents();
	}
}
