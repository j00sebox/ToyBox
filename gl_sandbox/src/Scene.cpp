#include "pch.h"
#include "Scene.h"

#include "Renderer.h"
#include "ParseFile.h"
#include "GLTFLoader.h"

#include "components/Transform.h"
#include "components/Light.h"
#include "components/Mesh.h"
#include "components/Material.h"

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
	if (scene == "")
		return;

	std::string src = file_to_string(scene);

	m_json = json::parse(src);

	json camera = m_json["camera"];
	json camera_pos = camera["position"];

	m_camera->set_pos(mathz::Vec3({ camera_pos[0], camera_pos[1], camera_pos[2] }));

	std::string skybox_src = m_json.value("skybox", "");

	if (!skybox_src.empty())
	{
		Skybox sb(skybox_src);

		ShaderProgram sp(
			Shader("resources/shaders/skybox/skybox_vertex.shader", ShaderType::Vertex),
			Shader("resources/shaders/skybox/skybox_fragment.shader", ShaderType::Fragment)
		);

		sb.attach_shader_program(std::move(sp));

		m_skybox = std::make_unique<Skybox>(std::move(sb));
	}

	json shaders = m_json["shaders"];
	unsigned int shader_count = m_json["shader_count"];

	for (unsigned int s = 0; s < shader_count; ++s)
	{
		// TODO: add support for other shader types
		std::string vertex_src = shaders[s]["vertex"];
		Shader vertex_shader(vertex_src.c_str(), ShaderType::Vertex);

		std::string fragment_src = shaders[s]["fragment"];
		Shader fragment_shader(fragment_src.c_str(), ShaderType::Fragment);

		ShaderProgram shader_program(vertex_shader, fragment_shader);

		std::string shader_name = shaders[s]["name"];
		ShaderLib::add(shader_name, std::move(shader_program));
	}

	unsigned int model_count = m_json["model_count"];
	json models = m_json["models"];

	for (unsigned int i = 0; i < model_count; ++i)
	{
		Model m;

		json model = models[i];

		m.set_name(model.value("name", "no_name"));

		Transform t{};
		json info = model["transform"];

		json translation = info["translate"];
		t.translate(mathz::Vec3({ translation[0], translation[1], translation[2] }));

		json rotation = info["rotation"];
		t.rotate(rotation[0], { rotation[1], rotation[2], rotation[3] });

		t.scale(info["scale"]);

		m.attach(std::move(t));

		if (!model["light"].is_null())
		{
			std::string type = model["light"]["type"];

			if (type == "point_light")
			{
				PointLight pl;

				json colour = model["light"]["colour"];
				pl.set_colour(mathz::Vec4(colour[0], colour[1], colour[2], colour[3]));
				pl.set_radius(model["light"]["radius"]);
				pl.set_brightness(model["light"]["brightness"]);

				m.attach(std::move(pl));
			}
			else if (type == "directional_light")
			{
				DirectionalLight dl;

				json colour = model["light"]["colour"];
				dl.set_colour(mathz::Vec4(colour[0], colour[1], colour[2], colour[3]));
				
				json dir = model["light"]["direction"];
				dl.set_direction({ dir[0], dir[1], dir[2] });

				dl.set_brightness(model["light"]["brightness"]);

				m.attach(std::move(dl));
			}
		}

		if (!model["gltf"].is_null())
		{
			GLTFLoader loader = m.load_gltf(model["gltf"]["path"]); 

			Mesh mesh;
			mesh.load(loader);
			m.attach(std::move(mesh));

			Material material;
			material.load(loader);
			material.set_shader(ShaderLib::get(model["shader"]));
			
			m.attach(std::move(material));
		}
		else if (!model["primitive"].is_null())
		{
			if (model["primitive"] == "cube")
			{
				Mesh mesh;
				mesh.load_primitive(PrimitiveTypes::Cube);
				m.attach(std::move(mesh));

				Material material;
				material.set_shader(ShaderLib::get(model["shader"]));
				material.set_colour({ 1.f, 1.f, 1.f, 1.f });
				m.attach(std::move(material));
			}
		}
	
		m_entities.emplace_back(std::make_unique<Model>(std::move(m)));
	}
}

void Scene::init(int width, int height)
{
	m_camera->resize(width, height);

	if (m_skybox)
	{
		m_skybox->get_shader()->set_uniform_mat4f("u_projection", m_camera->get_perspective());
	}

	for (unsigned int i = 0; i < m_entities.size(); ++i)
	{
		if (m_entities[i]->has<Material>())
		{
			auto& material = m_entities[i]->get<Material>();
			material.get_shader()->set_uniform_mat4f("u_projection", m_camera->get_perspective());
		}
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
	
	for (unsigned int i = 0; i < m_entities.size(); ++i)
	{
		ImGui::BeginChild("##LeftSide", ImVec2(120, ImGui::GetContentRegionAvail().y), true);
		{
			bool selected = false;
			ImGui::Selectable(m_entities[i]->get_name().c_str(), &selected);
			if (selected)
			{
				m_selected_entity = m_entities[i].get();
			}
		}
		ImGui::EndChild();
		
		auto& transform = m_entities[i]->get<Transform>();

		if (m_entities[i]->has<PointLight>())
		{
			auto& point_light = m_entities[i]->get<PointLight>();
			mathz::Vec3 pos = transform.get_transform() * transform.get_position();

			ShaderLib::get("pbr_standard")->set_uniform_4f(std::format("point_lights[{}].colour", point_light.get_index()), point_light.get_colour());
			ShaderLib::get("pbr_standard")->set_uniform_1f(std::format("point_lights[{}].brightness", point_light.get_index()), point_light.get_brightness());
			ShaderLib::get("pbr_standard")->set_uniform_3f(std::format("point_lights[{}].position", point_light.get_index()), pos);
			ShaderLib::get("pbr_standard")->set_uniform_1f(std::format("point_lights[{}].radius", point_light.get_index()), point_light.get_radius());
			ShaderLib::get("pbr_standard")->set_uniform_1f(std::format("point_lights[{}].range", point_light.get_index()), point_light.get_range());
			ShaderLib::get("pbr_standard")->set_uniform_3f("u_cam_pos", m_camera->get_pos());
			ShaderLib::get("pbr_standard")->set_uniform_4f("u_emissive_colour", point_light.get_colour());
		}
		else if (m_entities[i]->has<DirectionalLight>())
		{
			auto& direct_light = m_entities[i]->get<DirectionalLight>();

			ShaderLib::get("pbr_standard")->set_uniform_4f("directional_light.colour", direct_light.get_colour());
			ShaderLib::get("pbr_standard")->set_uniform_1f("directional_light.brightness", direct_light.get_brightness());
			ShaderLib::get("pbr_standard")->set_uniform_3f("directional_light.direction", direct_light.get_direction());
			ShaderLib::get("pbr_standard")->set_uniform_3f("u_cam_pos", m_camera->get_pos());
			ShaderLib::get("pbr_standard")->set_uniform_4f("u_emissive_colour", direct_light.get_colour());
		}
		else
		{
			ShaderLib::get("pbr_standard")->set_uniform_4f("u_emissive_colour", {0.f, 0.f, 0.f, 0.f});
		}

		if (m_entities[i]->has<Mesh>())
		{
			auto& material = m_entities[i]->get<Material>();
			auto& mesh = m_entities[i]->get<Mesh>();

			material.get_shader()->set_uniform_mat4f("u_model", transform.get_transform());
			material.get_shader()->set_uniform_mat4f("u_view", m_camera->camera_look_at());
			material.bind();
			mesh.bind();
			Renderer::draw_elements(mesh.get_index_count());
			mesh.unbind();
			material.unbind();
		}
	}

	{
		ImGui::SameLine(0);
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine();
	}

	ImGui::BeginChild("##RightSide", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true);
	{
		if (m_selected_entity)
		{
			ImGui::Text(m_selected_entity->get_name().c_str());

			render_components();
		}
	}
	ImGui::EndChild();

	ImGui::End();
}

void Scene::render_components()
{
	std::vector<std::shared_ptr<Component>> components = m_selected_entity->get_components();

	for (unsigned int i = 0; i < components.size(); ++i)
	{
		ImVec2 content_region = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 5, 5 });
		float line_width = GImGui->Font->FontSize + GImGui->Style.FramePadding.x * 3.0f;
		float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y;
		ImGui::PopStyleVar();

		if(ImGui::TreeNodeEx(components[i]->get_name(), ImGuiTreeNodeFlags_DefaultOpen))
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
				m_selected_entity->remove(*components[i--]);
			}

			components[i]->imgui_render();
			ImGui::TreePop();
		}	
	}
}

void Scene::reset_view()
{
	m_camera->reset();
}

