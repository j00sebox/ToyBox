#include "pch.h"
#include "SceneSerializer.h"

#include "GLTFLoader.h"
#include "ParseFile.h"

#include "Entity.h"
#include "Camera.h"
#include "Skybox.h"

#include "components/Transform.h"
#include "components/Light.h"
#include "components/Mesh.h"
#include "components/Material.h"

#include <mathz/Matrix.h>
#include <mathz/Quaternion.h>

void SceneSerializer::open(const char* scene, std::shared_ptr<Camera>& camera, std::unique_ptr<Skybox>& sky_box, std::vector<std::unique_ptr<Entity>>& entities)
{
	if (scene == "")
		return;

	std::string src = file_to_string(scene);

	json w_json = json::parse(src);

	json camera_accessor = w_json["camera"];
	json camera_pos = camera_accessor["position"];

	camera->set_pos(mathz::Vec3({ camera_pos[0], camera_pos[1], camera_pos[2] }));

	load_skybox(w_json, sky_box);

	json shaders = w_json["shaders"];
	unsigned int shader_count = w_json["shader_count"];

	load_shaders(shaders, shader_count);

	json models = w_json["models"];
	unsigned int model_count = w_json["model_count"];

	load_models(models, model_count, entities);
}

void SceneSerializer::load_skybox(json accessor, std::unique_ptr<Skybox>& sky_box)
{
	std::string skybox_src = accessor.value("skybox", "");

	if (!skybox_src.empty())
	{
		Skybox sb(skybox_src);

#ifdef PLATFORM_WINDOWS
		const char* v_path = "resources/shaders/skybox/skybox_vertex.shader";
#else
		const char* v_path = "./gl_sandbox/resources/shaders/skybox/skybox_vertex.shader";
#endif

#ifdef PLATFORM_WINDOWS
		const char* f_path = "resources/shaders/skybox/skybox_fragment.shader";
#else
		const char* f_path = "./gl_sandbox/resources/shaders/skybox/skybox_fragment.shader";
#endif

		ShaderProgram sp(
			Shader(v_path, ShaderType::Vertex),
			Shader(f_path, ShaderType::Fragment)
		);

		sb.attach_shader_program(std::move(sp));

		sky_box = std::make_unique<Skybox>(std::move(sb));
	}
}

void SceneSerializer::load_shaders(nlohmann::json accessor, unsigned int num_shaders)
{
	for (unsigned int s = 0; s < num_shaders; ++s)
	{
		// TODO: add support for other shader types
		std::string vertex_src = accessor[s]["vertex"];
		Shader vertex_shader(vertex_src.c_str(), ShaderType::Vertex);

		std::string fragment_src = accessor[s]["fragment"];
		Shader fragment_shader(fragment_src.c_str(), ShaderType::Fragment);

		ShaderProgram shader_program(vertex_shader, fragment_shader);

		std::string shader_name = accessor[s]["name"];
		ShaderLib::add(shader_name, std::move(shader_program));
	}
}

void SceneSerializer::load_models(nlohmann::json accessor, unsigned int num_models, std::vector<std::unique_ptr<Entity>>& entities)
{
	for (unsigned int i = 0; i < num_models; ++i)
	{
		Entity e;

		json model = accessor[i];

		e.set_name(model.value("name", "no_name"));

		Transform t{};
		json info = model["transform"];

		json translation = info["translate"];
		t.translate(mathz::Vec3({ translation[0], translation[1], translation[2] }));

		json rotation = info["rotation"];
		t.rotate(rotation[0], { rotation[1], rotation[2], rotation[3] });

		t.scale(info["scale"]);

		e.attach(std::move(t));

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

				e.attach(std::move(pl));
			}
			else if (type == "directional_light")
			{
				DirectionalLight dl;

				json colour = model["light"]["colour"];
				dl.set_colour(mathz::Vec4(colour[0], colour[1], colour[2], colour[3]));

				json dir = model["light"]["direction"];
				dl.set_direction({ dir[0], dir[1], dir[2] });

				dl.set_brightness(model["light"]["brightness"]);

				e.attach(std::move(dl));
			}
			}

		if (!model["gltf"].is_null())
		{
			std::string gltf_path = model["gltf"]["path"];
			GLTFLoader loader(gltf_path.c_str());

			Mesh mesh;
			mesh.load(loader);
			e.attach(std::move(mesh));

			Material material;
			material.load(loader);
			material.set_shader(ShaderLib::get(model["shader"]));

			e.attach(std::move(material));
		}
		else if (!model["primitive"].is_null())
		{
			if (model["primitive"] == "cube")
			{
				Mesh mesh;
				mesh.load_primitive(PrimitiveTypes::Cube);
				e.attach(std::move(mesh));

				Material material;
				material.set_shader(ShaderLib::get(model["shader"]));
				material.set_colour({ 1.f, 1.f, 1.f, 1.f });
				e.attach(std::move(material));
			}
		}

		entities.emplace_back(std::make_unique<Entity>(std::move(e)));
	}
}