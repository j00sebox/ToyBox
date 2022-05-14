#include "pch.h"
#include "SceneSerializer.h"

#include "GLTFLoader.h"
#include "FileOperations.h"

#include "Entity.h"
#include "Camera.h"
#include "Skybox.h"

#include "components/Transform.h"
#include "components/Light.h"
#include "components/Mesh.h"
#include "components/Material.h"

#include <mathz/Matrix.h>
#include <mathz/Quaternion.h>

static std::vector<mathz::Vec3> floats_to_vec3(const std::vector<float>& flts);
static std::vector<mathz::Vec2<float>> floats_to_vec2(const std::vector<float>& flts);

void SceneSerializer::open(const char* scene, std::shared_ptr<Camera>& camera, std::unique_ptr<Skybox>& sky_box, std::vector<std::unique_ptr<Entity>>& entities)
{
	if (scene == "")
		return;

	std::string src = file_to_string(scene);

	json w_json = json::parse(src);                                            

	// Not sure where to put this
	PointLight::m_point_light_count = 0;

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

void SceneSerializer::save(const char* scene, std::shared_ptr<Camera>& camera, std::unique_ptr<Skybox>& sky_box, std::vector<std::unique_ptr<Entity>>& entities)
{
	json res_json;

	if (sky_box)
	{
		res_json["skybox"] = sky_box->get_resource_path();
	}

	mathz::Vec3 camera_pos = camera->get_pos();
	res_json["camera"]["position"][0] = camera_pos.x;
	res_json["camera"]["position"][1] = camera_pos.y;
	res_json["camera"]["position"][2] = camera_pos.z;

	size_t num_shader = ShaderLib::get_num();
	res_json["shader_count"] = num_shader;

	int i = 0;
	for (const auto& [name, shader_ptr] : ShaderLib::m_shaders)
	{
		res_json["shaders"][i]["name"] = name;

		// TODO: Find better way to do this
		std::vector<std::string> locations = shader_ptr->get_shader_locations();
		res_json["shaders"][i]["vertex"] = locations[0];
		res_json["shaders"][i]["fragment"] = locations[1];
		++i;
	}
	
	res_json["model_count"] = entities.size();

	int j = 0;
	for (const auto& e : entities)
	{	
		res_json["models"][j]["name"] = e->get_name();
		
		if (e->has<Material>())
		{
			Material& material = e->get<Material>();
			res_json["models"][j]["shader"] = ShaderLib::find(material.get_shader());
		}

		const auto& components = e->get_components();

		for (const auto& c : components)
		{
			c->serialize(res_json["models"][j]);
		}
		 
		++j;
	}

	overwrite_file(scene, res_json.dump());
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
	auto load_gltf_mesh = [](const GLTFLoader& loader, Mesh& mesh) 
	{
		std::vector<mathz::Vec3> positions = floats_to_vec3(loader.get_positions());
		std::vector<mathz::Vec3> normals = floats_to_vec3(loader.get_normals());
		std::vector<mathz::Vec2<float>> tex_coords = floats_to_vec2(loader.get_tex_coords());

		std::vector<Vertex> vertices;

		for (unsigned int i = 0; i < positions.size(); i++)
		{
			vertices.push_back({
					positions[i],
					normals[i],
					tex_coords[i]
				});
		}

		std::vector<float> verts;

		for (const Vertex& v : vertices)
		{
			verts.push_back(v.positon.x);
			verts.push_back(v.positon.y);
			verts.push_back(v.positon.z);
			verts.push_back(v.normal.x);
			verts.push_back(v.normal.y);
			verts.push_back(v.normal.z);
			verts.push_back(v.st.x);
			verts.push_back(v.st.y);
		}

		std::vector<unsigned int> indices = loader.get_indices();

		mesh.load(verts, indices);
	};

	auto load_gltf_material = [](const GLTFLoader& loader, Material& material)
	{
		std::string textures[4] = {
			loader.get_base_color_texture(),
			loader.get_specular_texture(),
			loader.get_normal_texture(),
			loader.get_occlusion_texture()
		};

		material.load(textures);
	};

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
			load_gltf_mesh(loader, mesh);
			
			// TODO: remove later
			mesh.m_gltf_path = model["gltf"]["path"];
			e.attach(std::move(mesh));

			Material material;
			load_gltf_material(loader, material);
			material.set_shader(ShaderLib::get(model["shader"]));

			e.attach(std::move(material));
		}
		else if (!model["primitive"].is_null())
		{
			std::string p_name = model["primitive"];

			Mesh mesh;
			mesh.load_primitive(str_to_primitive_type(p_name.c_str()));
			e.attach(std::move(mesh));

			Material material;
			material.set_shader(ShaderLib::get(model["shader"]));
			material.set_colour({ 1.f, 1.f, 1.f, 1.f });
			e.attach(std::move(material));
		}

		entities.emplace_back(std::make_unique<Entity>(std::move(e)));
	}
}

std::vector<mathz::Vec3> floats_to_vec3(const std::vector<float>& flts)
{
	std::vector<mathz::Vec3> vec;
	for (unsigned int i = 0; i < flts.size();)
	{
		vec.push_back({ flts[i++], flts[i++], flts[i++] });
	}
	
	return vec;
}

std::vector<mathz::Vec2<float>> floats_to_vec2(const std::vector<float>& flts)
{
	std::vector<mathz::Vec2<float>> vec;
	for (unsigned int i = 0; i < flts.size();)
	{
		vec.push_back({ flts[i++], flts[i++] });
	}

	return vec;
}