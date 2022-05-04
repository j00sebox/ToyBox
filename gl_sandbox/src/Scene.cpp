#include "pch.h"
#include "Scene.h"

#include "ParseFile.h"

#include "components/Transform.h"
#include "components/Light.h"

#include "mathz/Misc.h"

#include <imgui.h>
#include <imgui_internal.h>

Scene::Scene()
{
	m_directional_light = { 2.f, -1.f, -1.f };
	m_directional_light.normalize();
	m_camera = std::make_shared<Camera>();
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

	json directional_light = m_json["directional_light"];
	m_directional_light = mathz::Vec3({ directional_light[0], directional_light[1], directional_light[2] });
	m_directional_light.normalize();

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
		m_shader_lib.add(shader_name, std::move(shader_program));
	}

	unsigned int model_count = m_json["model_count"];
	json models = m_json["models"];

	for (unsigned int i = 0; i < model_count; ++i)
	{
		Model m;

		json model = models[i];
		
		m.set_name(model.value("name", "no_name"));

		unsigned int mesh_count = model["mesh_count"];
		json meshes = model["meshes"];

		for (unsigned int j = 0; j < mesh_count; ++j)
		{
			m.load_mesh(meshes[j]["path"]);
		}

		m.set_shader(model["shader"]);

		unsigned int component_count = model["component_count"];
		json components = model["components"];

		for (unsigned int i = 0; i < model_count; ++i)
		{
			std::string type = components[i]["type"];
			
			if (type == "transform")
			{
				Transform t{};
				t.parse(components[i]);
				m.attach(t);
			}
		}

		m_entities.emplace_back(std::make_unique<Model>(std::move(m)));
	}
}

void Scene::init()
{
	if (m_skybox)
	{
		m_skybox->get_shader()->set_uniform_mat4f("u_projection", m_camera->get_perspective());
	}
	
	// TODO: move to scene file
	m_entities.emplace_back(std::make_unique<Model>());
	m_entities[1]->attach(Transform());
	m_entities[1]->attach(PointLight());
	m_entities[1]->set_shader("point_light");
	m_entities[1]->set_name("point light");
	PointLight& pl = m_entities[1]->get<PointLight>();
	m_shader_lib.get("point_light")->set_uniform_4f("u_light_colour", pl.get_colour());

	for (unsigned int i = 0; i < m_entities.size(); ++i)
	{
		m_shader_lib.get(m_entities[i]->get_shader())->set_uniform_mat4f("u_projection", m_camera->get_perspective());
	}
}

void Scene::update(float elapsed_time)
{
	m_camera->update(elapsed_time);
}

void Scene::draw()
{
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

		Transform& t = m_entities[i]->get<Transform>();

		m_shader_lib.get(m_entities[i]->get_shader())->set_uniform_mat4f("u_model", t.get_transform());
		m_shader_lib.get(m_entities[i]->get_shader())->set_uniform_mat4f("u_view", m_camera->camera_look_at());
		m_shader_lib.get(m_entities[i]->get_shader())->bind();
		m_entities[i]->draw();
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

			m_selected_entity->render_components();
		}
	}
	ImGui::EndChild();

	ImGui::End();
}
