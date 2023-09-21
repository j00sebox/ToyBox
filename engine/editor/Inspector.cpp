#include "Inspector.hpp"
#include "Entity.hpp"
#include "Component.h"
#include "Input.h"

#include <imgui.h>
#include <imgui_internal.h>

void Inspector::render()
{
    ImGui::Begin("Inspector");
    ImGui::BeginChild("##LeftSide", ImVec2(200, ImGui::GetContentRegionAvail().y), true);

    for (auto* sceneNode : m_scene->root)
    {
        imguiRender(sceneNode);
    }

    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    ImGui::BeginChild("##RightSide", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true);
    {
        if (m_scene->selectedNode)
        {
            char buf[64];
            strcpy(buf, m_scene->selectedNode->get_name().c_str());
            if (ImGui::InputText("##EntityName", buf, IM_ARRAYSIZE(buf)))
            {
                m_scene->selectedNode->set_name(buf);
            }

            displayComponents();
        }
    }
    ImGui::EndChild();

    ImGui::End();

    if (dragNode && !dropNode && !Input::is_button_pressed(GLFW_MOUSE_BUTTON_1))
    {
        dropNode = &m_scene->root;
    }

    if (dragNode && dropNode)
    {
        dropNode->move_child(dragNode);
        m_scene->selectedNode = nullptr;
        dragNode = nullptr;
        dropNode = nullptr;
    }
}

void Inspector::imguiRender(SceneNode* currentNode)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap;
    if (!currentNode->has_children()) flags |= ImGuiTreeNodeFlags_Leaf;
    bool opened = ImGui::TreeNodeEx(currentNode->get_name().c_str(), flags);

    if (ImGui::IsItemClicked())
    {
        m_scene->selectedNode = currentNode;
    }

    if (ImGui::BeginPopupContextItem())
    {
        m_scene->selectedNode = currentNode;

        if (ImGui::MenuItem("Delete"))
        {
            m_scene->m_nodes_to_remove.push(currentNode);
        }

        if (ImGui::BeginMenu("Add Component"))
        {
//            if (ImGui::MenuItem("Point Light"))
//            {
//                scene->selectedNode->entity()->add_component(PointLight{});
//                scene->m_light_manager.add_point_light(*scene->selectedNode);
//            }

            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("_TREENODE", nullptr, 0);
        dragNode = currentNode;
        ImGui::TextUnformatted(currentNode->get_name().c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (ImGui::AcceptDragDropPayload("_TREENODE"))
        {
            dropNode = currentNode;
        }
        ImGui::EndDragDropTarget();
    }

    if (opened)
    {
        for (auto* node : *currentNode)
        {
            imguiRender(node);
        }
        ImGui::TreePop();
    }
}

void Inspector::displayComponents() const
{
    std::vector<std::shared_ptr<Component>> components = m_scene->selectedNode->get_components();

    for (auto& component : components)
    {
        ImVec2 content_region = ImGui::GetContentRegionAvail();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{5, 5});
        float line_width = GImGui->Font->FontSize + GImGui->Style.FramePadding.x * 3.0f;
        float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y;
        ImGui::PopStyleVar();

        if (ImGui::TreeNodeEx(component->get_name(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool remove_component = false;

            ImGui::SameLine(content_region.x - line_width);

            if (ImGui::Button("...", ImVec2{line_width, line_height}))
            {
                ImGui::OpenPopup("component_settings");
            }

            if (ImGui::BeginPopup("component_settings"))
            {
                if (ImGui::MenuItem("Remove component"))
                {
                    remove_component = true;
                }

                ImGui::EndPopup();
            }

            // FIXME
//            if (remove_component)
//            {
//                if (scene->selectedNode->entity()->has_component<PointLight>())
//                    scene->m_light_manager.remove_point_light(scene->selectedNode);
//
//                scene->selectedNode->entity()->remove_component(*component);
//            }

            component->imgui_render();
            ImGui::TreePop();
        }
    }
}

