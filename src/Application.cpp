#include "pch.h"
#include "Application.h"

#include "Input.h"

#include <imgui.h>

void Application::start()
{
	m_running = true;

	m_current_scene->load("../resources/scenes/spooky.scene");

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

		//ImGui::ShowDemoWindow();
		display_dockspace();
		m_window.display_render_context();
		display_menu();
		display_fps();

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

void Application::display_dockspace()
{
	static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	ImGui::Begin("DockSpace", &m_show_dock_space, window_flags);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("DSP_ID");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }

	ImGui::PopStyleVar(2);

	ImGui::End();
}

void Application::display_menu()
{
	ImGui::Begin("Menu", (bool*)true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Scene"))
		{
			if (ImGui::BeginMenu("Open"))
			{
				for (const auto& entry : std::filesystem::directory_iterator("../resources/scenes/"))
				{
					if (ImGui::MenuItem(entry.path().filename().string().c_str()))
					{
						switch_scene(entry.path().string().c_str());
					}
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
					std::string path = std::string("../resources/scenes/") + std::string(buf);
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
				m_current_scene->add_primitive("cube");
			}

			if (ImGui::MenuItem("Quad"))
			{
				m_current_scene->add_primitive("quad");
			}

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	ImGui::End();
}

void Application::display_fps()
{
	ImGui::Begin("FPS");
	ImGui::Text("Avg. %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}

