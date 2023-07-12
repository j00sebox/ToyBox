#include "pch.h"
#include "SceneSerializer.h"
#include "GLTFLoader.h"
#include "FileOperations.h"
#include "Entity.h"
#include "Camera.h"
#include "Skybox.h"
#include "Scene.h"
#include "SceneNode.h"
#include "Mesh.h"
#include "components/Transform.h"
#include "components/Light.h"
#include "components/MeshComponent.h"
#include "renderer/Material.h"

static std::vector<glm::vec3> floats_to_vec3(const std::vector<float>& flts);
static std::vector<glm::vec2> floats_to_vec2(const std::vector<float>& flts);

void SceneSerializer::open(const char* scene_name, Scene& scene, std::shared_ptr<Camera>& camera, std::unique_ptr<Skybox>& sky_box, SceneNode& root)
{
	if (!strcmp(scene_name, ""))
		return;

	std::string src = file_to_string(scene_name);

	json w_json = json::parse(src);                                            

	json camera_accessor = w_json["camera"];
	json camera_pos = camera_accessor["position"];
    json camera_fwd = camera_accessor["forward"];
	camera->set_pos(glm::vec3(camera_pos[0], camera_pos[1], camera_pos[2]));
    camera->set_forward(glm::vec3(camera_fwd[0], camera_fwd[1], camera_fwd[2]));

    if(!w_json["background_colour"].is_null())
    {
        json bg_col = w_json["background_colour"];
        scene.set_background_colour({bg_col[0], bg_col[1], bg_col[2], bg_col[3]});
    }

    if(!w_json["skybox"].is_null())
        load_skybox(w_json["skybox"], sky_box);

	json models = w_json["models"];
	unsigned int model_count = w_json["model_count"];

	load_models(models, model_count, root, scene);
}

void SceneSerializer::save(const char* scene_name, const Scene& scene, const std::shared_ptr<Camera>& camera, const std::unique_ptr<Skybox>& sky_box, const SceneNode& root)
{
	json res_json;

	if (sky_box)
	{
		res_json["skybox"]["path"] = sky_box->get_resource_path();
        res_json["skybox"]["image_format"] = (int)sky_box->get_image_format();
	}

	glm::vec3 camera_pos = camera->get_pos();
	res_json["camera"]["position"][0] = camera_pos.x;
	res_json["camera"]["position"][1] = camera_pos.y;
	res_json["camera"]["position"][2] = camera_pos.z;

    glm::vec3 camera_fwd = camera->get_forward();
    res_json["camera"]["forward"][0] = camera_fwd.x;
    res_json["camera"]["forward"][1] = camera_fwd.y;
    res_json["camera"]["forward"][2] = camera_fwd.z;

	glm::vec4 bg_col = scene.get_background_colour();
	res_json["background_colour"][0] = bg_col.x;
	res_json["background_colour"][1] = bg_col.y;
	res_json["background_colour"][2] = bg_col.z;
	res_json["background_colour"][3] = bg_col.w;

	int node_index = -1; // keeps track of nodes (left to right and depth first)
	for (const auto& scene_node : root)
	{
		++node_index;
		serialize_node(res_json["models"], node_index, scene_node);
	}

	res_json["model_count"] = node_index + 1;

	overwrite_file(scene_name, res_json.dump());
}

void SceneSerializer::serialize_node(json& accessor, int& node_index, const SceneNode& scene_node)
{
	accessor[node_index]["name"] = scene_node.entity->get_name();

	if (scene_node.entity->has_component<Material>())
	{
		auto& material = scene_node.entity->get_component<Material>();
		accessor[node_index]["shader"] = ShaderTable::find(material.get_shader());
	}

	const auto& components = scene_node.entity->get_components();

	for (const auto& c : components)
	{
		c->serialize(accessor[node_index]);
	}

	if (scene_node.has_children())
	{
		int ch_ind = 0; // keeps track of index of child array
		int parent_index = node_index;
		for (const SceneNode& child : scene_node)
		{
			accessor[parent_index]["children"][ch_ind++] = ++node_index;
			serialize_node(accessor, node_index, child);
		}
		accessor[parent_index]["child_count"] = scene_node.size();
	}
}

void SceneSerializer::load_skybox(const json& accessor, std::unique_ptr<Skybox>& sky_box)
{
    sky_box = std::make_unique<Skybox>(accessor["path"], accessor["image_format"]);
}

void SceneSerializer::load_models(const json& accessor, unsigned int model_count, SceneNode& root, Scene& scene)
{
	int num_models_checked = 0;
	while (num_models_checked < model_count)
	{
		root.add_child(load_model(accessor, num_models_checked, num_models_checked, scene));
	}
}

// TODO: Make this look less ugly
SceneNode SceneSerializer::load_model(const json& accessor, int model_index, int& num_models_checked, Scene& scene)
{
    auto load_gltf_mesh = [](const GLTFLoader& loader, Mesh& mesh)
    {
        std::vector<glm::vec3> positions = floats_to_vec3(loader.get_positions());
        std::vector<glm::vec3> normals = floats_to_vec3(loader.get_normals());
        std::vector<glm::vec2> tex_coords = floats_to_vec2(loader.get_tex_coords());

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
            verts.push_back(v.position.x);
            verts.push_back(v.position.y);
            verts.push_back(v.position.z);
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

	Entity entity;

	json model = accessor[model_index];

    entity.set_name(model.value("name", "no_name"));

	Transform transform{};
	json info = model["transform"];

	json translation = info["translate"];
    transform.translate(glm::vec3({ translation[0], translation[1], translation[2] }));

	json rotation = info["rotation"];
    transform.rotate(rotation[0], { rotation[1], rotation[2], rotation[3] });

    transform.scale(info["scale"]);

    glm::mat4 model_matrix = transform.get_transform();
    glm::vec3 position = transform.get_position();
    entity.add_component(std::move(transform));

	if (!model["light"].is_null())
	{
		std::string type = model["light"]["type"];

		if (type == "point_light")
		{
			PointLight pl;

			json colour = model["light"]["colour"];
			pl.set_colour(glm::vec4(colour[0], colour[1], colour[2], colour[3]));
			pl.set_range(model["light"]["range"]);
			pl.set_brightness(model["light"]["brightness"]);

            entity.add_component(std::move(pl));
		}
		else if (type == "directional_light")
		{
			DirectionalLight dl;

			json colour = model["light"]["colour"];
			dl.set_colour(glm::vec4(colour[0], colour[1], colour[2], colour[3]));

			json dir = model["light"]["direction"];
			dl.set_direction({ dir[0], dir[1], dir[2] });

			dl.set_brightness(model["light"]["brightness"]);

            if(model["light"]["cast_shadow"])
            {
                dl.cast_shadow();
                dl.shadow_init(position);
            }

            entity.add_component(std::move(dl));
		}
	}

    if(!model["mesh"].is_null())
    {
        MeshComponent mesh_component;
        GLTFLoader loader;

        json mesh_accessor = model["mesh"];

        mesh_component.m_mesh_type = mesh_accessor["mesh_type"];
        std::string mesh_name = mesh_accessor["mesh_name"];

        if(!MeshTable::exists(mesh_name))
        {
            Mesh mesh;

            if(mesh_accessor["mesh_type"] == "gltf")
            {
                loader.read_file(mesh_name.c_str());
                load_gltf_mesh(loader, mesh);
            }
            else if(mesh_accessor["mesh_type"] == "primitive")
            {
                mesh.load_primitive(str_to_primitive_type(mesh_name.c_str()));
            }

            MeshTable::add(mesh_name, std::move(mesh));
        }

        mesh_component.m_use_scale_outline = mesh_accessor["use_scale_outline"];
        mesh_component.m_outlining_factor = mesh_accessor["outlining_factor"];

        mesh_component.set_mesh(MeshTable::get(mesh_name));
        mesh_component.set_mesh_name(mesh_name);

        if(mesh_accessor["instanced"])
        {
            scene.instanced_meshes[mesh_name].push_back(model_matrix);
            mesh_component.m_instance_id = scene.instanced_meshes[mesh_name].size() - 1;
        }

        entity.add_component(std::move(mesh_component));

        if(!model["material"].is_null())
        {
            json material_accessor = model["material"];
            TexturingMode texturing_mode = (TexturingMode)material_accessor["texturing_mode"];

            // if the mesh is being instanced then make one material that can be shared by all those instances
            std::string mat_name = (mesh_accessor["instanced"]) ? mesh_name : entity.get_name();

            if(!MaterialTable::exists(mat_name))
            {
                Material material;

                material.set_colour({ material_accessor["properties"]["colour"][0], material_accessor["properties"]["colour"][1], material_accessor["properties"]["colour"][2], material_accessor["properties"]["colour"][3]});
                material.set_metallic_property(material_accessor["properties"]["metallic_property"]);
                material.set_roughness(material_accessor["properties"]["roughness"]);

                if(texturing_mode == TexturingMode::MODEL_DEFAULT)
                {
                    load_gltf_material(loader, material);
                }
                else
                {
                    std::string base_colour_location = (material_accessor["textures"]["base_colour"] == "") ? "../resources/textures/white_on_white.jpeg" : material_accessor["textures"]["base_colour"];

                    std::string textures[] = {
                            base_colour_location,
                            material_accessor["textures"]["specular"],
                            material_accessor["textures"]["normal_map"],
                            material_accessor["textures"]["occlusion"]
                    };

                    material.load(textures);
                }

                // FIXME
                if(mesh_accessor["instanced"])
                    material.set_shader(ShaderTable::get("inst_default"));
                else
                    material.set_shader(ShaderTable::get(material_accessor["shader"]));

                MaterialTable::add(mat_name, std::move(material));
            }

            MaterialComponent material_component(MaterialTable::get(mat_name));
            material_component.set_texturing_mode(texturing_mode);

            entity.add_component(std::move(material_component));
        }
    }

	SceneNode current_node{ std::make_unique<Entity>(std::move(entity)) };

	int num_children = accessor[model_index].value("child_count", 0);

	for (int i = 0; i < num_children; ++i)
	{
		int child_index = accessor[model_index]["children"][i];
		current_node.add_child(load_model(accessor, child_index, num_models_checked, scene));
	}

	++num_models_checked;
	return current_node;
}

void SceneSerializer::load_mesh(nlohmann::json& mesh_accessor, MeshComponent& mesh_object)
{
    auto load_gltf_mesh = [](const GLTFLoader& loader, Mesh& mesh)
    {
        std::vector<glm::vec3> positions = floats_to_vec3(loader.get_positions());
        std::vector<glm::vec3> normals = floats_to_vec3(loader.get_normals());
        std::vector<glm::vec2> tex_coords = floats_to_vec2(loader.get_tex_coords());

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
            verts.push_back(v.position.x);
            verts.push_back(v.position.y);
            verts.push_back(v.position.z);
            verts.push_back(v.normal.x);
            verts.push_back(v.normal.y);
            verts.push_back(v.normal.z);
            verts.push_back(v.st.x);
            verts.push_back(v.st.y);
        }

        std::vector<unsigned int> indices = loader.get_indices();

        mesh.load(verts, indices);
    };

    std::string mesh_name = mesh_accessor["mesh_name"];

    if(!MeshTable::exists(mesh_name))
    {
        Mesh mesh;

        if(mesh_accessor["mesh_type"] == "gltf")
        {
            GLTFLoader loader(mesh_name.c_str());
            load_gltf_mesh(loader, mesh);
        }
        else if(mesh_accessor["mesh_type"] == "primitive")
        {
            mesh.load_primitive(str_to_primitive_type(mesh_name.c_str()));
        }

        MeshTable::add(mesh_name, std::move(mesh));
    }

    mesh_object.set_mesh(MeshTable::get(mesh_name));
    mesh_object.set_mesh_name(mesh_name);
}


void SceneSerializer::load_material(nlohmann::json& material_accessor, Material& material)
{
    material.set_colour({ material_accessor["properties"]["colour"][0], material_accessor["properties"]["colour"][1], material_accessor["properties"]["colour"][2], material_accessor["properties"]["colour"][3]});
    material.set_metallic_property(material_accessor["properties"]["metallic_property"]);
    material.set_roughness(material_accessor["properties"]["roughness"]);

    std::string base_colour_location = (material_accessor["textures"]["base_colour"] == "") ? "../resources/textures/white_on_white.jpeg" : material_accessor["textures"]["base_colour"];

    std::string textures[] = {
            base_colour_location,
            material_accessor["textures"]["specular"],
            material_accessor["textures"]["normal_map"],
            material_accessor["textures"]["occlusion"]
    };

    material.load(textures);
}

std::vector<glm::vec3> floats_to_vec3(const std::vector<float>& flts)
{
	std::vector<glm::vec3> vec;
	for (unsigned int i = 0; i < flts.size();)
	{
		vec.emplace_back(glm::vec3{ flts[i++], flts[i++], flts[i++] });
	}
	
	return vec;
}

std::vector<glm::vec2> floats_to_vec2(const std::vector<float>& flts)
{
	std::vector<glm::vec2> vec;
	for (unsigned int i = 0; i < flts.size();)
	{
		vec.emplace_back(glm::vec2{ flts[i++], flts[i++] });
	}

	return vec;
}