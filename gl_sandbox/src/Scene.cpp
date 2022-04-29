#include "pch.h"
#include "Scene.h"

#include "ParseFile.h"

void Scene::set_perspective(const mathz::Mat4& perspective)
{
	m_perspective = perspective;
}

void Scene::load(const char* scene)
{
	std::string src = file_to_string(scene);

	m_json = json::parse(src);

	std::string skybox_src = m_json.value("skybox", "");

	if (!skybox_src.empty())
	{
		Skybox sb(skybox_src);

		ShaderProgram sp(
			Shader("resources/shaders/skybox/skybox_vertex.shader", ShaderType::Vertex),
			Shader("resources/shaders/skybox/skybox_fragment.shader", ShaderType::Fragment)
		);

		sp.set_uniform_mat4f("u_projection", m_perspective);
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

		json shaders = model["shaders"];

		std::string vertex_src = shaders["vertex"];
		Shader vertex_shader(vertex_src.c_str(), ShaderType::Vertex);

		std::string fragment_src = shaders["fragment"];
		Shader fragment_shader(fragment_src.c_str(), ShaderType::Fragment);

		ShaderProgram shader_program(vertex_shader, fragment_shader);

		shader_program.set_uniform_mat4f("u_model", mathz::Mat4());
		shader_program.set_uniform_mat4f("u_projection", m_perspective);
		shader_program.set_uniform_3f("u_light", mathz::Vec3({ 0.f, 0.f, -1.f }));

		m.attach_shader_program(std::move(shader_program));

		m_models.emplace_back(std::move(m));
	}
}

void Scene::draw(const mathz::Mat4& look_at, const mathz::Mat4& look_at_no_t)
{
	if (m_skybox)
	{
		m_skybox->get_shader()->set_uniform_mat4f("u_view", look_at_no_t);
		m_skybox->draw();
	}

	for (const Model& model : m_models)
	{
		model.get_shader()->set_uniform_mat4f("u_view", look_at);
		model.draw();
	}
}
