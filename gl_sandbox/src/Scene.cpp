#include "pch.h"
#include "Scene.h"

#include "ParseFile.h"

#include "lights/PointLight.h"

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

		json translate = model["translate"];
		m.translate(mathz::Vec3({ translate[0], translate[1], translate[2] }));

		json rotation = model["rotation"];
		m.rotate(rotation[0], { rotation[1], rotation[2], rotation[3] });

		m.scale(model["scale"]);

		m_objects.emplace_back(std::make_unique<Model>(std::move(m)));
	}
}

void Scene::init()
{
	if (m_skybox)
	{
		m_skybox->get_shader()->set_uniform_mat4f("u_projection", m_camera->get_perspective());
	}
	
	m_objects.emplace_back(std::make_unique<PointLight>());
	m_shader_lib.get("point_light")->set_uniform_4f("u_light_colour", { { 1.f, 1.f, 1.f }, 1.f });

	for (unsigned int i = 0; i < m_objects.size(); ++i)
	{
		m_shader_lib.get(m_objects[i]->get_shader())->set_uniform_mat4f("u_projection", m_camera->get_perspective());
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
	
	for (unsigned int i = 0; i < m_objects.size(); ++i)
	{
		ImGui::BeginChild("##LeftSide", ImVec2(120, ImGui::GetContentRegionAvail().y), true);
		{
			bool selected = false;
			ImGui::Selectable(m_objects[i]->get_name().c_str(), &selected);
			if (selected)
			{
				m_selected_object = m_objects[i].get();
			}
		}
		ImGui::EndChild();

		m_shader_lib.get(m_objects[i]->get_shader())->set_uniform_mat4f("u_model", m_objects[i]->get_transform());
		m_shader_lib.get(m_objects[i]->get_shader())->set_uniform_mat4f("u_view", m_camera->camera_look_at());
		m_shader_lib.get(m_objects[i]->get_shader())->bind();
		m_objects[i]->draw();
	}

	{
		ImGui::SameLine(0);
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine();
	}

	ImGui::BeginChild("##RightSide", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true);
	{
		if (m_selected_object)
		{
			ImGui::Text(m_selected_object->get_name().c_str());
			
			mathz::Vec3 position = m_selected_object->get_position();
			ImGui::Text("\nPosition: ");
			ImGui::InputFloat("x", &position.x);
			ImGui::InputFloat("y", &position.y);
			ImGui::InputFloat("z", &position.z);
			m_selected_object->translate(position);
			
			float angle = m_selected_object->get_rotate_angle();
			mathz::Vec3 axis = m_selected_object->get_rotate_axis();
			ImGui::Text("\nRotation: ");
			ImGui::InputFloat("angle", &angle);
			ImGui::SliderFloat("i", &axis.x, -1.f, 1.f);
			ImGui::SliderFloat("j", &axis.y, -1.f, 1.f);
			ImGui::SliderFloat("k", &axis.z, -1.f, 1.f);
			axis.normalize();
			m_selected_object->rotate(angle, axis);
		}
	}
	ImGui::EndChild();

	ImGui::End();
}
