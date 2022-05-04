#include "pch.h"
#include "Scene.h"

#include "ParseFile.h"
#include "GLTFLoader.h"

#include "components/Transform.h"
#include "components/Light.h"
#include "components/Mesh.h"
#include "components/Material.h"

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

		Transform t{};
		t.parse(model["transform"]);
		m.attach(std::move(t));

		if (!model["light"].is_null())
		{
			std::string type = model["light"]["type"];

			if (type == "point_light")
			{
				PointLight pl;

				json colour = model["light"]["colour"];
				pl.set_colour(mathz::Vec4({ {colour[0], colour[1], colour[2]}, colour[3] }));

				m.attach(std::move(pl));
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
			material.set_shader(m_shader_lib.get(model["shader"]));
			m.attach(std::move(material));
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
	/*m_entities.emplace_back(std::make_unique<Model>());
	m_entities[1]->attach(Transform());
	m_entities[1]->attach(PointLight());
	m_entities[1]->set_name("point light");*/

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

		auto& transform = m_entities[i]->get<Transform>();

		if (m_entities[i]->has<PointLight>())
		{
			auto& point_light = m_entities[i]->get<PointLight>();
			mathz::Vec3 pos = transform.get_transform() * transform.get_position();

			m_shader_lib.get("texture2D")->set_uniform_4f("u_pl_col", point_light.get_colour());
			m_shader_lib.get("texture2D")->set_uniform_3f("u_pl_pos", pos);
		}

		if (m_entities[i]->has<Material>())
		{
			auto& material = m_entities[i]->get<Material>();

			material.get_shader()->set_uniform_mat4f("u_model", transform.get_transform());
			material.get_shader()->set_uniform_mat4f("u_view", m_camera->camera_look_at());
			m_entities[i]->draw();
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

			m_selected_entity->render_components();
		}
	}
	ImGui::EndChild();

	ImGui::End();
}
