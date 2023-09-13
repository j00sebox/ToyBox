#include "pch.h"
#include "Application.h"
#include "Input.h"
#include "Timer.h"
#include "Log.h"

#include <imgui.h>

void Application::start()
{
	m_running = true;

    {
        info("Beginning startup process...\n");
        Timer t;
        currentScene->load("../assets/scenes/test.scene");
        currentScene->init();
        auto [width, height] = m_window.get_dimensions();
        Renderer::init(width, height);
    }

	while (m_running)
	{
		if (Input::is_key_pressed(GLFW_KEY_ESCAPE))
		{
			m_running = false;
			continue;
		}

        if(Input::is_key_pressed(GLFW_KEY_C))
        {
            Scene::recompile_shaders();
        }

		m_window.begin_frame();

        display_dockspace();

		float delta_time = m_window.get_delta_time();
        inspector.render();
        currentScene->update(delta_time);

		//ImGui::ShowDemoWindow();
		m_window.display_render_context();
		display_menu();
		display_fps();

		m_window.end_frame();
	}

    shutdown();
}

void Application::switch_scene(const char* scene_path)
{
    info("Switching scene...\n");
    Timer t;
    delete currentScene;
    currentScene = new Scene(&m_window);
    currentScene->load(scene_path);
    currentScene->init();
    inspector.scene = currentScene;
}

void Application::shutdown()
{
    delete currentScene;
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
	static bool display_bg_col_picker = false;

	ImGui::Begin("Menu", (bool*)true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Scene"))
		{
			if (ImGui::BeginMenu("Open"))
			{
				for (const auto& entry : std::filesystem::directory_iterator("../assets/scenes/"))
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
				static char buf[32] = R"(test.scene)";
				ImGui::Text("assets/scenes/");
				ImGui::SameLine();
				ImGui::InputText("##LeftSide", buf, IM_ARRAYSIZE(buf));

				if (ImGui::Button("Save As"))
				{
					std::string path = std::string("../assets/scenes/") + std::string(buf);
                    currentScene->save(path);
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Add"))
		{
			if (ImGui::MenuItem("Cube"))
			{
                currentScene->add_primitive("cube");
			}

			if (ImGui::MenuItem("Quad"))
			{
                currentScene->add_primitive("quad");
			}

            if (ImGui::BeginMenu("Open Model"))
            {
                if (ImGui::BeginMenu("Choose model to open"))
                {
                    static char buf[128] = "";
                    ImGui::InputText("##LeftSide", buf, IM_ARRAYSIZE(buf));

                    if (ImGui::Button("Open"))
                    {
                        std::string path = std::string(buf);
                        currentScene->add_model(path.c_str());
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu("Settings"))
		{
			if(ImGui::MenuItem("Set Background Colour"))
			{
				display_bg_col_picker = true;
			}

            if(ImGui::MenuItem("V-Sync Toggle"))
            {
                m_window.toggle_vsync();
            }

            if(ImGui::BeginMenu("Anti-Aliasing"))
            {
                if(ImGui::MenuItem("No AA"))
                {
                    m_window.change_sample_amount(1);
                }

                if(ImGui::MenuItem("MSAA X4"))
                {
                    m_window.change_sample_amount(4);
                }

				if (ImGui::MenuItem("MSAA X6"))
				{
					m_window.change_sample_amount(6);
				}

                ImGui::EndMenu();
            }

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();

		if(display_bg_col_picker)
		{
			ImGui::Begin("Background Colour");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x);
			if(ImGui::Button("x"))
			{
				display_bg_col_picker = false;
			}
			glm::vec4 bg_colour = currentScene->get_background_colour();
			float col[4] = { bg_colour.x, bg_colour.y, bg_colour.z, bg_colour.w };
			if(ImGui::ColorPicker4("##BGColor", col))
			{
                currentScene->set_background_colour({ col[0], col[1], col[2], col[3] });
			}
			ImGui::End();
		}
	}
	ImGui::End();
}

void Application::display_fps()
{
	ImGui::Begin("FPS");
	ImGui::Text("Avg. %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}

