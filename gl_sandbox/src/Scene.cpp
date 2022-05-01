#include "pch.h"
#include "Scene.h"

#include "ParseFile.h"

#include "lights/PointLight.h"

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

	m_camera->set_pos(mathz::Vec3({camera_pos[0], camera_pos[1], camera_pos[2]}));

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

	unsigned int model_count = m_json["model_count"];

	json models = m_json["models"];

	for (unsigned int i = 0; i < model_count; ++i)
	{
		Model m;

		json model = models[i];
		unsigned int mesh_count = model["mesh_count"];
		
		json meshes = model["meshes"];

		for (unsigned int j = 0; j < mesh_count; ++j)
		{
			m.load_mesh(meshes[j]["path"]);
		}

		json translate = model["translate"];
		m.translate(mathz::Vec3({ translate[0], translate[1], translate[2] }));

		json rotation = model["rotation"];
		m.rotate(mathz::Quaternion(rotation[0], rotation[1], rotation[2], rotation[3]));

		m.scale(model["scale"]);

		json shaders = model["shaders"];

		std::string vertex_src = shaders["vertex"];
		Shader vertex_shader(vertex_src.c_str(), ShaderType::Vertex);

		std::string fragment_src = shaders["fragment"];
		Shader fragment_shader(fragment_src.c_str(), ShaderType::Fragment);

		ShaderProgram shader_program(vertex_shader, fragment_shader);

		m.attach_shader_program(std::move(shader_program));

		m_models.emplace_back(std::move(m));
	}
}

void Scene::init()
{
	if (m_skybox)
	{
		m_skybox->get_shader()->set_uniform_mat4f("u_projection", m_camera->get_perspective());
	}

	m_point_light_shader = m_point_light.get_shader();

	m_point_light.translate({ 0.f, 0.f, -9.f });

	m_point_light_shader->set_uniform_mat4f("u_model", m_point_light.get_translate());
	m_point_light_shader->set_uniform_mat4f("u_projection", m_camera->get_perspective());
	m_point_light_shader->set_uniform_4f("u_light_colour", m_point_light.get_colour());

	Model floor;
	floor.load_mesh("resources/models/wooden_floor/scene.gltf");

	floor.scale(0.05f);
	floor.translate({ 100.f, 0.f, -200.f });
	//floor.rotate(mathz::Quaternion(90.f, { 1.f, 0.f, 0.f }));

	ShaderProgram sp(
		Shader("resources/shaders/texture2D/texture2D_vertex.shader", ShaderType::Vertex),
		Shader("resources/shaders/texture2D/texture2D_fragment.shader", ShaderType::Fragment)
	);

	floor.attach_shader_program(std::move(sp));

	m_models.emplace_back(std::move(floor));

	for (const Model& model : m_models)
	{
		model.get_shader()->set_uniform_mat4f("u_model", model.get_transform());
		model.get_shader()->set_uniform_mat4f("u_projection", m_camera->get_perspective());
		model.get_shader()->set_uniform_3f("u_pl_pos", m_point_light.get_postion());
	//	model.get_shader()->set_uniform_4f("u_pl_col", m_point_light.get_colour());
	//	model.get_shader()->set_uniform_1f("u_pl_rad", 50.f);
	//	model.get_shader()->set_uniform_3f("u_directional_light", m_directional_light);
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

	m_point_light_shader->set_uniform_mat4f("u_view", m_camera->camera_look_at());
//	m_point_light_shader->set_uniform_3f("u_camera_position", m_camera->get_pos());
	m_point_light.draw();

	for (const Model& model : m_models)
	{
		model.get_shader()->set_uniform_mat4f("u_view", m_camera->camera_look_at());
		model.draw();
	}
}
