#include "pch.h"
#include "MenuBar.hpp"
#include "scene/SceneSerializer.hpp"

#include <imgui.h>
#include <spdlog/fmt/bundled/format.h>

void MenuBar::display(Scene* scene, Renderer* renderer)
{
    ImGui::Begin("Menu", (bool*)true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Scene"))
        {
            display_scene_dropdown(scene);
        }

        if (ImGui::BeginMenu("Add"))
        {
            display_add_dropdown(scene, renderer);
        }

        if (ImGui::BeginMenu("Settings"))
        {
            display_settings_dropdown();
        }
        ImGui::EndMenuBar();
    }
    ImGui::End();
}

void MenuBar::display_scene_dropdown(const Scene* current_scene)
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
            SceneSerializer::save(path.c_str(), current_scene);
        }

        ImGui::EndMenu();
    }
    ImGui::EndMenu();
}

void MenuBar::display_add_dropdown(Scene* current_scene, Renderer* renderer)
{
    if (ImGui::MenuItem("Cube"))
    {
        auto* new_node = new SceneNode();
        SceneSerializer::load_primitive(new_node, "cube", renderer);
        set_unique_name(new_node, current_scene->root);
        current_scene->root->add_child(new_node);
    }

    if (ImGui::MenuItem("Quad"))
    {
        auto* new_node = new SceneNode();
        SceneSerializer::load_primitive(new_node, "quad", renderer);
        set_unique_name(new_node, current_scene->root);
        current_scene->root->add_child(new_node);
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
                auto* new_node = new SceneNode();
                SceneSerializer::load_model(new_node, path.c_str(), renderer);
                set_unique_name(new_node, current_scene->root);
                new_node->add_component(Transform{});
                current_scene->root->add_child(new_node);
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

bool MenuBar::name_exists(const std::string& name, const SceneNode* scene_node)
{
    if(name == scene_node->get_name())
    {
        return true;
    }

    for(auto* child_node : scene_node->children)
    {
        if(name_exists(name, child_node))
        {
            return true;
        }
    }

    return false;
}

void MenuBar::set_unique_name(SceneNode* scene_node, const SceneNode* root)
{
    if(name_exists(scene_node->get_name(), root))
    {
        u32 index = 1;
        std::string unique_name = scene_node->get_name() + fmt::format(" ({})", index);
        while(name_exists(unique_name, root))
        {
            unique_name = scene_node->get_name() + fmt::format(" ({})", index);
            ++index;
        }
        scene_node->set_name(unique_name);
    }
}
