#include "pch.h"
#include "Scene.h"

#include "Entity.h"
#include "Renderer.h"
#include "SceneSerializer.h"

#include "components/Transform.h"
#include "components/Light.h"
#include "components/Mesh.h"
#include "components/Material.h"

#include "events/EventList.h"
#include "profiler/Timer.h"

#include <mathz/Misc.h>

#include <imgui.h>
#include <imgui_internal.h>

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
	SceneSerializer::open(scene, m_camera, m_skybox, root);
}

void Scene::save(const std::string& path)
{
	SceneSerializer::save(path.c_str(), m_camera, m_skybox, root);
}

void Scene::init(int width, int height)
{
	EventList::e_resize.bind_function(std::bind(&Scene::window_resize, this, std::placeholders::_1, std::placeholders::_2));
	m_camera->resize(width, height);
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
			ImGui::Text(m_selected_node->entity->get_name().c_str());

			display_components();
		}
	}
	ImGui::EndChild();

	ImGui::End();

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
		lookup = std::string(name) + std::format(" ({})", i);
		++i;
	}

	Entity e;
	e.set_name(lookup.c_str());
	e.attach(Transform());
	
	Mesh mesh;
	mesh.load_primitive(str_to_primitive_type(name));
	e.attach(std::move(mesh));

	Material material;
	material.set_shader(ShaderLib::get("pbr_standard"));
	material.set_colour({ 1.f, 1.f, 1.f, 1.f });
	e.attach(std::move(material));

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
	if (!root.remove(node))
	{
		fprintf(stderr, "Node not apart of current scene tree!");
	}
	else
	{
		m_selected_node = nullptr;
	}
}

void Scene::update_node(SceneNode& scene_node, const Transform& parent_transform)
{
	auto transform = scene_node.entity->get<Transform>();
	Transform t = parent_transform * transform;

	update_lights(scene_node);

	if (scene_node.entity->has<Mesh>())
	{
		auto& material = scene_node.entity->get<Material>();
		auto& mesh = scene_node.entity->get<Mesh>();

		material.get_shader()->set_uniform_mat4f("u_model", t.get_transform());
		material.get_shader()->set_uniform_mat4f("u_view", m_camera->camera_look_at());
		material.get_shader()->set_uniform_mat4f("u_projection", m_camera->get_perspective());

		if (m_selected_node && (scene_node == *m_selected_node))
		{
			t.scale(t.get_uniform_scale() * 1.1f); // scale up a tiny bit to see outline
			ShaderLib::get("flat_colour")->set_uniform_mat4f("u_model", t.get_transform());
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
		update_node(node, t);
	}
}

void Scene::update_lights(SceneNode& light_node)
{
	if (light_node.entity->has<PointLight>())
	{
		auto transform = light_node.entity->get<Transform>();
		auto& point_light = light_node.entity->get<PointLight>();
		mathz::Vec3 pos = transform.get_transform() * transform.get_position();

		ShaderLib::get("pbr_standard")->set_uniform_4f(std::format("point_lights[{}].colour", point_light.get_index()), point_light.get_colour());
		ShaderLib::get("pbr_standard")->set_uniform_1f(std::format("point_lights[{}].brightness", point_light.get_index()), point_light.get_brightness());
		ShaderLib::get("pbr_standard")->set_uniform_3f(std::format("point_lights[{}].position", point_light.get_index()), pos);
		ShaderLib::get("pbr_standard")->set_uniform_1f(std::format("point_lights[{}].radius", point_light.get_index()), point_light.get_radius());
		ShaderLib::get("pbr_standard")->set_uniform_1f(std::format("point_lights[{}].range", point_light.get_index()), point_light.get_range());
		ShaderLib::get("pbr_standard")->set_uniform_3f("u_cam_pos", m_camera->get_pos());
		ShaderLib::get("pbr_standard")->set_uniform_4f("u_emissive_colour", point_light.get_colour());
	}
	else if (light_node.entity->has<DirectionalLight>())
	{
		auto& direct_light = light_node.entity->get<DirectionalLight>();

		ShaderLib::get("pbr_standard")->set_uniform_4f("directional_light.colour", direct_light.get_colour());
		ShaderLib::get("pbr_standard")->set_uniform_1f("directional_light.brightness", direct_light.get_brightness());
		ShaderLib::get("pbr_standard")->set_uniform_3f("directional_light.direction", direct_light.get_direction());
		ShaderLib::get("pbr_standard")->set_uniform_3f("u_cam_pos", m_camera->get_pos());
		ShaderLib::get("pbr_standard")->set_uniform_4f("u_emissive_colour", direct_light.get_colour());
	}
	else
	{
		ShaderLib::get("pbr_standard")->set_uniform_4f("u_emissive_colour", { 0.f, 0.f, 0.f, 0.f });
	}
}

void Scene::imgui_render(SceneNode& scene_node)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap;
	if (!scene_node.has_children()) flags |= ImGuiTreeNodeFlags_Leaf;
	bool opened = ImGui::TreeNodeEx(scene_node.get_name().c_str(), flags);
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

		ImGui::EndPopup();
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
				m_selected_node->entity->remove(*components[i--]);
			}

			components[i]->imgui_render();
			ImGui::TreePop();
		}
	}
}

