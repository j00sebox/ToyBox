#include "pch.h"
#include "Application.h"

#include "Input.h"

#include <imgui.h>

void Application::start()
{
	m_running = true;

#ifdef PLATFORM_WINDOWS
	m_current_scene->load("resources/scenes/spooky.scene");
#else
	m_current_scene->load("./toy_box/resources/scenes/spooky.scene");
#endif
	auto [width, height] = m_window.get_dimensions();
	m_current_scene->init(width, height);
	Renderer::init(width, height);

	while (m_running)
	{
		if (Input::is_key_pressed(GLFW_KEY_ESCAPE))
		{
			m_running = false;
			continue;
		}

		m_window.begin_frame();

		ImGui::ShowDemoWindow();

		display_menu();

		float delta_time = m_window.get_delta_time();
		m_current_scene->update(delta_time);

		m_window.end_frame();
	}
}

void Application::switch_scene(const char* scene_path)
{
	m_current_scene.reset(new Scene());
	m_current_scene->load(scene_path);
	auto [width, height] = m_window.get_dimensions();
	m_current_scene->init(width, height);
}

void Application::display_menu()
{
	ImGui::Begin("Menu", (bool*)1, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Scene"))
		{
			if (ImGui::BeginMenu("Open"))
			{
				// TODO: Make this more dynamic
				if (ImGui::MenuItem("Flying High"))
				{
#ifdef PLATFORM_WINDOWS
					switch_scene("resources/scenes/flying_high.scene");
#else
					switch_scene("./toy_box/resources/scenes/flying_high.scene");
#endif
				}

				if (ImGui::MenuItem("Spooky"))
				{
#ifdef PLATFORM_WINDOWS
					switch_scene("resources/scenes/spooky.scene");
#else
					switch_scene("./toy_box/resources/scenes/spooky.scene");
#endif
				}

				if (ImGui::MenuItem("Something"))
				{
#ifdef PLATFORM_WINDOWS
					switch_scene("resources/scenes/something.scene");
#else
					switch_scene("./toy_box/resources/scenes/spooky.scene");
#endif
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Save"))
			{
				static char buf[32] = "\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x2E\x73\x63\x65\x6E\x65";
				ImGui::Text("resources/scenes/");
				ImGui::SameLine();
				ImGui::InputText("##LeftSide", buf, IM_ARRAYSIZE(buf));

				if (ImGui::Button("Save As"))
				{
					std::string path = std::string("resources/scenes/") + std::string(buf);
					m_current_scene->save(path);
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}	
		if (ImGui::BeginMenu("Add"))
		{
			if (ImGui::MenuItem("Cube"))
			{
				m_current_scene->add_cube();
			}

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	ImGui::End();
}
