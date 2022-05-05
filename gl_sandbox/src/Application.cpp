#include "pch.h"
#include "Application.h"

#include "Input.h"

void Application::start()
{
	m_running = true;
	m_current_scene.load("resources/scenes/spooky.scene");
	auto [width, height] = m_window.get_dimensions();
	m_current_scene.init(width, height);
	Renderer::init(width, height);

	while (m_running)
	{
		if (Input::is_key_pressed(GLFW_KEY_ESCAPE))
		{
			m_running = false;
			continue;
		}

		m_window.begin_frame();

		float delta_time = m_window.get_delta_time();
		m_current_scene.update(delta_time);

		m_window.end_frame();
	}
}
