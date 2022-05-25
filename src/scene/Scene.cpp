#include "pch.h"
#include "Scene.h"

#include "Entity.h"
#include "Renderer.h"
#include "Input.h"
#include "SceneSerializer.h"

#include "components/Transform.h"
#include "components/Light.h"
#include "components/Mesh.h"
#include "components/Material.h"

#include "events/EventList.h"
#include "profiler/Timer.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <mathz/Misc.h>
#include <spdlog/fmt/bundled/format.h>

Scene::Scene()
{
	m_camera = std::make_shared<Camera>();
}

Scene::~Scene()
{
	ShaderLib::release();
}

void Scene::load(const char* scene)
{
	SceneSerializer::open(scene, *this, m_camera, m_skybox, root);
}

void Scene::save(const std::string& path)
{
	SceneSerializer::save(path.c_str(), *this, m_camera, m_skybox, root);
}

void Scene::init(int width, int height)
{
	EventList::e_resize.bind_function(std::bind(&Scene::window_resize, this, std::placeholders::_1, std::placeholders::_2));
	m_camera->resize(width, height);

	for (const SceneNode& node : root)
	{
		m_light_manager.set_lights(node);
	}
}

void Scene::update(float elapsed_time)
{
	Renderer::clear();

	m_camera->update(elapsed_time);

	if (m_skybox)
	{
		m_skybox->get_shader()->set_uniform_mat4f("u_projection", m_camera->get_perspective());
		m_skybox->get_shader()->set_uniform_mat4f("u_view", m_camera->look_at_no_translate());
		m_skybox->draw();
	}

	ImGui::Begin("Models");
	
	ImGui::BeginChild("##LeftSide", ImVec2(200, ImGui::GetContentRegionAvail().y), true);

	m_light_manager.update_lights(m_camera);
	for (auto& scene_node : root)
	{
		update_node(scene_node, Transform{});
		imgui_render(scene_node);
	}	

	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
	ImGui::SameLine();

	ImGui::BeginChild("##RightSide", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true);
	{
		if (m_selected_node)
		{
			char buf[32];
			strcpy(buf, m_selected_node->entity->get_name().c_str());
			if (ImGui::InputText("##EntityName", buf, IM_ARRAYSIZE(buf)))
			{
				m_selected_node->entity->set_name(buf);
			}

			display_components();
		}
	}
	ImGui::EndChild();

	ImGui::End();

	// resolve drag and drop
	if (m_drag_node && !m_drop_node && !Input::is_button_pressed(GLFW_MOUSE_BUTTON_1))
	{
		m_drop_node = &root;
	}

	if (m_drag_node && m_drop_node)
	{
		m_drop_node->add_child(move_node(*m_drag_node));
		m_selected_node = nullptr;
		m_drag_node = nullptr;
		m_drop_node = nullptr;
	}

	while (!m_nodes_to_remove.empty())
	{
		remove_node(*m_nodes_to_remove.front());
		m_nodes_to_remove.pop();
	}
}

void Scene::add_primitive(const char* name)
{
	std::string lookup{ name };
	int i = 1;
	while (root.exists(lookup))
	{
		lookup = std::string(name) + fmt::format(" ({})", i);
		++i;
	}

	Entity e;
	e.set_name(lookup.c_str());
	e.add_component(Transform());
	
	Mesh mesh;
	mesh.load_primitive(str_to_primitive_type(name));
	e.add_component(std::move(mesh));

	Material material;
	material.set_shader(ShaderLib::get("pbr_standard"));
	material.set_colour({ 1.f, 1.f, 1.f, 1.f });
	e.add_component(std::move(material));

	if(m_selected_node)
		m_selected_node->add_child(std::make_unique<Entity>(std::move(e)));
	else
		root.add_child(std::make_unique<Entity>(std::move(e)));
}

void Scene::window_resize(int width, int height)
{
	m_camera->resize(width, height);
	Renderer::set_viewport(width, height);
}

void Scene::reset_view()
{
	m_camera->reset();
}

void Scene::remove_node(SceneNode& node)
{
	if (node.entity->has_component<PointLight>())
	{
		m_light_manager.remove_point_light(node);
	}

	if (!root.remove(node))
	{
		fprintf(stderr, "Node not apart of current scene tree!");
	}
	else
	{
		m_selected_node = nullptr;
	}
}

SceneNode Scene::move_node(SceneNode& node)
{
	return root.move(node);
}

void Scene::update_node(SceneNode& scene_node, const Transform& parent_transform)
{
	Transform relative_transform = scene_node.update(parent_transform);

	if (scene_node.entity->has_component<Mesh>())
	{
		auto& material = scene_node.entity->get_component<Material>();
		auto& mesh = scene_node.entity->get_component<Mesh>();

		material.get_shader()->set_uniform_mat4f("u_model", relative_transform.get_transform());
		material.get_shader()->set_uniform_mat4f("u_view", m_camera->camera_look_at());
		material.get_shader()->set_uniform_mat4f("u_projection", m_camera->get_perspective());

		if (m_selected_node && (scene_node == *m_selected_node))
		{
			Transform stencil_transform = relative_transform;
			stencil_transform.scale(stencil_transform.get_uniform_scale() * 1.03f); // scale up a tiny bit to see outline
			ShaderLib::get("flat_colour")->set_uniform_mat4f("u_model", stencil_transform.get_transform());
			ShaderLib::get("flat_colour")->set_uniform_mat4f("u_view", m_camera->camera_look_at());
			ShaderLib::get("flat_colour")->set_uniform_mat4f("u_projection", m_camera->get_perspective());

			Renderer::stencil(mesh, material);
		}
		else
		{
			Renderer::draw_elements(mesh, material);
		}
	}

	for (SceneNode& node : scene_node)
	{
		update_node(node, relative_transform);
	}
}

void Scene::imgui_render(SceneNode& scene_node)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap;
	if (!scene_node.has_children()) flags |= ImGuiTreeNodeFlags_Leaf;
	bool opened = ImGui::TreeNodeEx(scene_node.entity->get_name().c_str(), flags);

	if (ImGui::IsItemClicked())
	{
		m_selected_node = &scene_node;
	}

	if (ImGui::BeginPopupContextItem())
	{
		m_selected_node = &scene_node;
		
		if (ImGui::MenuItem("Delete"))
		{
			m_nodes_to_remove.push(&scene_node);
		}

		if (ImGui::BeginMenu("Add Component"))
		{
			if (ImGui::MenuItem("Point Light"))
			{
				m_selected_node->entity->add_component(PointLight{});
				m_light_manager.add_point_light(*m_selected_node);
			}

			ImGui::EndMenu();
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
		m_drag_node = &scene_node;
		ImGui::Text(scene_node.entity->get_name().c_str());
		ImGui::EndDragDropSource();
	}
	
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TREENODE"))
		{
			m_drop_node = &scene_node;
		}
		ImGui::EndDragDropTarget();
	}
	
	if (opened)
	{
		for (SceneNode& node : scene_node)
		{
			imgui_render(node);
		}
		ImGui::TreePop();
	}
}

void Scene::display_components()
{
	std::vector<std::shared_ptr<Component>> components = m_selected_node->entity->get_components();

	for (unsigned int i = 0; i < components.size(); ++i)
	{
		ImVec2 content_region = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 5, 5 });
		float line_width = GImGui->Font->FontSize + GImGui->Style.FramePadding.x * 3.0f;
		float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y;
		ImGui::PopStyleVar();

		if (ImGui::TreeNodeEx(components[i]->get_name(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			bool remove_component = false;

			ImGui::SameLine(content_region.x - line_width);

			if (ImGui::Button("...", ImVec2{ line_width, line_height }))
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

			if (remove_component)
			{
				if (m_selected_node->entity->has_component<PointLight>()) m_light_manager.remove_point_light(*m_selected_node);
				if (m_selected_node->entity->remove_component(*components[i])) --i;
			}

			components[i]->imgui_render();
			ImGui::TreePop();
		}
	}
}

void Scene::set_background_colour(mathz::Vec4 colour)
{
	m_clear_colour = colour;
	Renderer::set_clear_colour(colour);
}

