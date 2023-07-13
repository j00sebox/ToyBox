#include "Inspector.h"
#include "Entity.h"
#include "Component.h"
#include "Input.h"

#include <imgui.h>
#include <imgui_internal.h>

void Inspector::render()
{
    ImGui::Begin("Models");
    ImGui::BeginChild("##LeftSide", ImVec2(200, ImGui::GetContentRegionAvail().y), true);

    for (auto& sceneNode : *scene->root)
    {
        imguiRender(sceneNode);
    }

    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();

    ImGui::BeginChild("##RightSide", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true);
    {
        if (scene->selectedNode)
        {
            char buf[32];
            strcpy(buf, scene->selectedNode->entity()->get_name().c_str());
            if (ImGui::InputText("##EntityName", buf, IM_ARRAYSIZE(buf)))
            {
                scene->selectedNode->entity()->set_name(buf);
            }

            displayComponents();
        }
    }
    ImGui::EndChild();

    ImGui::End();

    if (dragNode && !dropNode && !Input::is_button_pressed(GLFW_MOUSE_BUTTON_1))
    {
        dropNode = scene->root;
    }

    if (dragNode && dropNode)
    {
        dropNode->move_child(dragNode);
        scene->selectedNode = nullptr;
        dragNode = nullptr;
        dropNode = nullptr;
    }
}

void Inspector::imguiRender(SceneNodePtr& currentNode)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap;
    if (!currentNode->has_children()) flags |= ImGuiTreeNodeFlags_Leaf;
    bool opened = ImGui::TreeNodeEx(currentNode->entity()->get_name().c_str(), flags);

    if (ImGui::IsItemClicked())
    {
        scene->selectedNode = currentNode;
    }

    if (ImGui::BeginPopupContextItem())
    {
        scene->selectedNode = currentNode;

        if (ImGui::MenuItem("Delete"))
        {
            scene->m_nodes_to_remove.push(currentNode);
        }

        if (ImGui::BeginMenu("Add Component"))
        {
            if (ImGui::MenuItem("Point Light"))
            {
                scene->selectedNode->entity()->add_component(PointLight{});
                scene->m_light_manager.add_point_light(*scene->selectedNode);
            }

            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("_TREENODE", nullptr, 0);
        dragNode = currentNode;
        ImGui::TextUnformatted(currentNode->entity()->get_name().c_str());
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
        for (auto& node : *currentNode)
        {
            imguiRender(node);
        }
        ImGui::TreePop();
    }
}

void Inspector::displayComponents()
{
    std::vector<std::shared_ptr<Component>> components = scene->selectedNode->entity()->get_components();

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
            if (remove_component)
            {
                if (scene->selectedNode->entity()->has_component<PointLight>())
                    scene->m_light_manager.remove_point_light(scene->selectedNode);

                scene->selectedNode->entity()->remove_component(*component);
            }

            component->imgui_render();
            ImGui::TreePop();
        }
    }
}

