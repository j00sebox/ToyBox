#include "pch.h"
#include "MenuBar.hpp"
#include "scene/SceneSerializer.hpp"

#include <imgui.h>

void MenuBar::display()
{
    ImGui::Begin("Menu", (bool*)true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Scene"))
        {
            display_scene_dropdown();
        }

        if (ImGui::BeginMenu("Add"))
        {
            display_add_dropdown();
        }

        if (ImGui::BeginMenu("Settings"))
        {
            display_settings_dropdown();
        }
        ImGui::EndMenuBar();
    }
    ImGui::End();
}

void MenuBar::display_scene_dropdown()
{
    if (ImGui::BeginMenu("Open"))
    {
        for (const auto& entry : std::filesystem::directory_iterator("../assets/scenes/"))
        {
            if (ImGui::MenuItem(entry.path().filename().string().c_str()))
            {
                // switch_scene(entry.path().string().c_str());
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
            SceneSerializer::save(path.c_str(), m_current_scene);
        }

        ImGui::EndMenu();
    }
    ImGui::EndMenu();
}

void MenuBar::display_add_dropdown()
{
    if (ImGui::MenuItem("Cube"))
    {
        // currentScene->add_primitive("cube");
    }

    if (ImGui::MenuItem("Quad"))
    {
        // currentScene->add_primitive("quad");
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
                // currentScene->add_model(path.c_str());
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    ImGui::EndMenu();
}

void MenuBar::display_settings_dropdown()
{
    if(ImGui::MenuItem("V-Sync Toggle"))
    {
        // m_window.toggle_vsync();
    }

    if(ImGui::BeginMenu("Anti-Aliasing"))
    {
        if(ImGui::MenuItem("No AA"))
        {
           // m_window.change_sample_amount(1);
        }

        if(ImGui::MenuItem("MSAA X4"))
        {
           // m_window.change_sample_amount(4);
        }

        if (ImGui::MenuItem("MSAA X6"))
        {
           // m_window.change_sample_amount(6);
        }

        ImGui::EndMenu();
    }
    ImGui::EndMenu();
}