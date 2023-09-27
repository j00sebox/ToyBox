#include "pch.h"
#include "SceneSerializer.hpp"
// #include "ModelLoader.h"
#include "Entity.hpp"
#include "Camera.hpp"
//#include "Skybox.h"
#include "Scene.hpp"
#include "SceneNode.hpp"
#include "Primitives.hpp"
// #include "Mesh.h"
#include "rendering/Renderer.hpp"
#include "components/Transform.h"
// #include "components/Light.h"
//#include "components/MeshComponent.h"
//#include "renderer/Material.h"
#include "util/FileOperations.hpp"
#include "util/ModelLoader.hpp"

#include <nlohmann/json.hpp>

enum class ImageFormat
{
    JPG = 0,
    PNG
};

const char* image_extension(ImageFormat fmt)
{
    switch (fmt)
    {
        case ImageFormat::JPG:
            return ".jpg";

        case ImageFormat::PNG:
            return ".png";
    }

    return "";
}

using namespace nlohmann;

void SceneSerializer::open(const char* scene_name, Scene* scene, Renderer* renderer)
{
	if (!strcmp(scene_name, ""))
    {
        return;
    }

	std::string src = fileop::file_to_string(scene_name);

	json w_json = json::parse(src);

	json camera_accessor = w_json["camera"];
	json camera_pos = camera_accessor["position"];
    json camera_fwd = camera_accessor["forward"];
	scene->camera->set_pos(glm::vec3(camera_pos[0], camera_pos[1], camera_pos[2]));
    scene->camera->set_forward(glm::vec3(camera_fwd[0], camera_fwd[1], camera_fwd[2]));

    if(!w_json["skybox"].is_null())
    {
        scene->skybox = std::make_shared<Skybox>(load_skybox(w_json["skybox"], renderer));
    }

	json models = w_json["models"];
	u32 model_count = w_json["model_count"];

    load_nodes(models, model_count, scene, renderer);
}

void SceneSerializer::save(const char* scene_name, const Scene* scene)
{
	json res_json;

	if (scene->skybox)
	{
		res_json["skybox"]["path"] = scene->skybox->resource_path;
        res_json["skybox"]["image_format"] = 0; // FIXME
	}

	glm::vec3 camera_pos = scene->camera->get_pos();
	res_json["camera"]["position"][0] = camera_pos.x;
	res_json["camera"]["position"][1] = camera_pos.y;
	res_json["camera"]["position"][2] = camera_pos.z;

    glm::vec3 camera_fwd = scene->camera->get_forward();
    res_json["camera"]["forward"][0] = camera_fwd.x;
    res_json["camera"]["forward"][1] = camera_fwd.y;
    res_json["camera"]["forward"][2] = camera_fwd.z;

	i32 node_index = -1; // keeps track of nodes (left to right and depth first)
	for (auto* scene_node : scene->root->children)
	{
		serialize_node(res_json["models"], ++node_index, scene_node);
	}

	res_json["model_count"] = ++node_index;

	fileop::overwrite_file(scene_name, res_json.dump());
}

void SceneSerializer::serialize_node(json& accessor, int& node_index, const SceneNode* scene_node)
{
	accessor[node_index]["name"] = scene_node->get_name();

//	if (scene_node->entity()->has_component<Material>())
//	{
//		auto& material = scene_node->entity()->get_component<Material>();
//		accessor[node_index]["shader"] = ShaderTable::find(material.get_shader());
//	}
	const auto& components = scene_node->get_components();

	for (const auto& component : components)
	{
		component->serialize(accessor[node_index]);
	}

    if(scene_node->is_model_root)
    {
        accessor[node_index]["model_root_path"] = scene_node->model_path.c_str();
        return;
    }

	if (scene_node->has_children())
	{
		int ch_ind = 0; // keeps track of index of child array
		int parent_index = node_index;
		for (const auto& child : scene_node->children)
		{
			accessor[parent_index]["children"][ch_ind++] = ++node_index;
			serialize_node(accessor, node_index, child);
		}
		accessor[parent_index]["child_count"] = scene_node->size();
	}
}

Skybox SceneSerializer::load_skybox(const json& accessor, Renderer* renderer)
{
    Skybox skybox{};

    std::string dir = accessor["path"];
    const char* img_ext = image_extension(accessor["image_format"]);
    std::string faces[6] = {
        dir + "right" + img_ext,
        dir + "left" + img_ext,
        dir + "top" + img_ext,
        dir + "bottom" + img_ext,
        dir + "front" + img_ext,
        dir + "back" + img_ext
    };

    skybox.resource_path = dir.c_str();

    skybox.cubemap = renderer->create_cubemap(faces);
    skybox.descriptor_set_layout = renderer->create_descriptor_set_layout({
        .bindings = {
            {
                .type = vk::DescriptorType::eCombinedImageSampler,
                .stage_flags = vk::ShaderStageFlagBits::eFragment,
            }
        },
        .num_bindings = 1
    });
    auto* skybox_descriptor_set_layout = renderer->get_descriptor_set_layout(skybox.descriptor_set_layout);
    skybox.descriptor_set = renderer->create_descriptor_set({
            .resource_handles = {skybox.cubemap},
            .sampler_handles = {k_invalid_sampler_handle},
            .bindings = {0},
            .types = {vk::DescriptorType::eCombinedImageSampler},
            .layout = skybox_descriptor_set_layout->vk_descriptor_set_layout,
            .num_resources = 1
    });

    std::vector<f32> skybox_verts =
    {
        -1.0f, -1.0f,  1.0f,	//        7--------6
        1.0f, -1.0f,  1.0f,	    //       /|       /|
        1.0f, -1.0f, -1.0f,	    //      4--------5 |
        -1.0f, -1.0f, -1.0f,	//      | |      | |
        -1.0f,  1.0f,  1.0f,	//      | 3------|-2
        1.0f,  1.0f,  1.0f,	    //      |/       |/
        1.0f,  1.0f, -1.0f, 	//      0--------1
        -1.0f,  1.0f, -1.0f
    };

    std::vector<u32> skybox_indices =
    {
        // right
        1, 2, 6,
        6, 5, 1,

        // left
        0, 4, 7,
        7, 3, 0,

        // top
        4, 5, 6,
        6, 7, 4,

        // bottom
        0, 3, 2,
        2, 1, 0,

        // back
        0, 1, 5,
        5, 4, 0,

        // front
        3, 7, 6,
        6, 2, 3
    };

    skybox.vertex_buffer = renderer->create_buffer({
        .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        .size = (u32)(sizeof(skybox_verts[0]) * skybox_verts.size()),
        .data = skybox_verts.data()
    });

    skybox.index_count = skybox_indices.size();
    skybox.index_buffer = renderer->create_buffer({
        .usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        .size = (u32)(sizeof(skybox_indices[0]) * skybox_indices.size()),
        .data = skybox_indices.data()
    });

    PipelineConfig pipeline_config{
            .renderPass = renderer->get_viewport_renderpass(),
            .pipeline_cache_location = "skybox_pipeline_cache.bin"
    };

    pipeline_config.set_rasterizer({
        .polygon_mode = vk::PolygonMode::eFill,
        .cull_mode = vk::CullModeFlagBits::eFront,
        .front_face = vk::FrontFace::eCounterClockwise
    });

    pipeline_config.set_binding_description({
        .binding = 0,
        .stride = sizeof(glm::vec3)
    });

    pipeline_config.set_input_assembly(vk::PrimitiveTopology::eTriangleList);

    pipeline_config.add_shader_stage({
        .shader_file = "../assets/shaders/skybox/vert.spv",
        .stage_flags = vk::ShaderStageFlagBits::eVertex
    });

    pipeline_config.add_shader_stage({
         .shader_file = "../assets/shaders/skybox/frag.spv",
         .stage_flags = vk::ShaderStageFlagBits::eFragment
    });

    pipeline_config.add_vertex_attribute({
        .location = 0,
        .format = vk::Format::eR32G32B32Sfloat,
        .offset = 0
    });

    pipeline_config.add_descriptor_set_layout(renderer->get_camera_data_layout());
    pipeline_config.add_descriptor_set_layout(skybox.descriptor_set_layout);

    vk::PipelineColorBlendAttachmentState colour_blend_attachment{};
    colour_blend_attachment.colorWriteMask = vk::ColorComponentFlagBits::eR
                                             | vk::ColorComponentFlagBits::eG
                                             | vk::ColorComponentFlagBits::eB
                                             | vk::ColorComponentFlagBits::eA;
    colour_blend_attachment.blendEnable = false;
    colour_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eOne;
    colour_blend_attachment.dstColorBlendFactor = vk::BlendFactor::eZero;
    colour_blend_attachment.colorBlendOp = vk::BlendOp::eAdd;
    colour_blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    colour_blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    colour_blend_attachment.alphaBlendOp = vk::BlendOp::eAdd;

    vk::PipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = vk::StructureType::ePipelineDepthStencilStateCreateInfo;
    depth_stencil.depthTestEnable = false;
    depth_stencil.depthWriteEnable = false;
    depth_stencil.depthCompareOp = vk::CompareOp::eLess;
    depth_stencil.depthBoundsTestEnable = false;
    depth_stencil.minDepthBounds = 0.f;
    depth_stencil.maxDepthBounds = 1.f;
    depth_stencil.stencilTestEnable = false;

    pipeline_config.add_colour_attachment(colour_blend_attachment);
    pipeline_config.add_depth_stencil_attachment(depth_stencil);

    skybox.pipeline = renderer->create_pipeline(pipeline_config);

    return skybox;
}

void SceneSerializer::load_nodes(const nlohmann::json &accessor, u32 model_count, Scene* scene, Renderer* renderer)
{
	u32 num_models_checked = 0;
	while (num_models_checked < model_count)
	{
        scene->root->add_child(load_node(accessor, num_models_checked, num_models_checked, scene, renderer));
	}
}

// TODO: Make this look less ugly
SceneNode* SceneSerializer::load_node(const nlohmann::json &accessor, u32 model_index, u32& num_models_checked, Scene* scene, Renderer* renderer)
{
    auto* current_node = new SceneNode();
	json model = accessor[model_index];

    current_node->set_name(model.value("name", "no_name"));

	Transform transform{};
	json info = model["transform"];

	json translation = info["translate"];
    transform.translate(glm::vec3({ translation[0], translation[1], translation[2] }));

	json rotation = info["rotation"];
    transform.rotate(rotation[0], { rotation[1], rotation[2], rotation[3] });

    transform.scale(info["scale"]);

    transform.recalculate_transform();
    glm::mat4 model_matrix = transform.get_transform();
    glm::vec3 position = transform.get_position();


//	if (!model["light"].is_null())
//	{
//		std::string type = model["light"]["type"];
//
//		if (type == "point_light")
//		{
//			PointLight pl;
//
//			json colour = model["light"]["colour"];
//			pl.set_colour(glm::vec4(colour[0], colour[1], colour[2], colour[3]));
//			pl.set_range(model["light"]["range"]);
//			pl.set_brightness(model["light"]["brightness"]);
//
//            entity.add_component(std::move(pl));
//		}
//		else if (type == "directional_light")
//		{
//			DirectionalLight dl;
//
//			json colour = model["light"]["colour"];
//			dl.set_colour(glm::vec4(colour[0], colour[1], colour[2], colour[3]));
//
//			json dir = model["light"]["direction"];
//			dl.set_direction({ dir[0], dir[1], dir[2] });
//
//			dl.set_brightness(model["light"]["brightness"]);
//
//            if(model["light"]["cast_shadow"])
//            {
//                dl.cast_shadow();
//                dl.shadow_init(position);
//            }
//
//            entity.add_component(std::move(dl));
//		}
//	}

    if(!model["model_root_path"].is_null())
    {
        std::string model_path = model["model_root_path"];
        ModelLoader model_loader(renderer, model_path.c_str());
        model_loader.load(current_node);
    }

    if(!model["mesh"].is_null())
    {
        json mesh_accessor = model["mesh"];
        std::string mesh_name = mesh_accessor["mesh_name"];

//        if(!MeshTable::exists(mesh_name))
//        {
//            Mesh mesh;
//
//            if ((mesh_accessor["mesh_type"] == "primitive"))
//            {
//                ModelLoader model_loader(str_to_primitive_type(mesh_name.c_str()));
//                model_loader.load_mesh(mesh);
//            }
//            else
//            {
//                ModelLoader model_loader(mesh_name.c_str());
//                model_loader.load_mesh(mesh);
//            }
//
//            MeshTable::add(mesh_name, std::move(mesh));
//        }

        load_model(current_node, mesh_name.c_str(), renderer);
//
//        ModelLoader model_loader(renderer, mesh_name.c_str());
//        model_loader.load(current_node);

        //mesh_component.set_mesh(MeshTable::get(mesh_name));


//        if(mesh_accessor["instanced"])
//        {
//            scene.instanced_meshes[mesh_name].push_back(model_matrix);
//            mesh_component.m_instance_id = (int)scene.instanced_meshes[mesh_name].size() - 1;
//        }

        // node.entity()->add_component(std::move(transform));

        //entity.add_component(std::move(mesh_component));

//        if(!model["material"].is_null())
//        {
//            json material_accessor = model["material"];
//            auto texturing_mode = (TexturingMode)material_accessor["texturing_mode"];
//
//            // if the mesh is being instanced then make one material that can be shared by all those instances
//            std::string mat_name = (mesh_accessor["instanced"]) ? mesh_name : entity.get_name();
//
//            if(!MaterialTable::exists(mat_name))
//            {
//                Material material;
//
//                material.set_colour({ material_accessor["properties"]["colour"][0], material_accessor["properties"]["colour"][1], material_accessor["properties"]["colour"][2], material_accessor["properties"]["colour"][3]});
//                material.set_metallic_property(material_accessor["properties"]["metallic_property"]);
//                material.set_roughness(material_accessor["properties"]["roughness"]);
//
//                if(texturing_mode == TexturingMode::MODEL_DEFAULT)
//                {
//                    ModelLoader model_loader(mesh_name.c_str());
//                    model_loader.load_material(material);
//                }
//                else
//                {
//                    std::string base_colour_location = (material_accessor["textures"]["base_colour"].empty()) ? "../assets/textures/white_on_white.jpeg" : material_accessor["textures"]["base_colour"];
//
//                    std::string textures[] = {
//                            base_colour_location,
//                            material_accessor["textures"]["specular"],
//                            material_accessor["textures"]["normal_map"],
//                            material_accessor["textures"]["occlusion"]
//                    };
//
//                    material.load(textures);
//                }
//
//                // FIXME
//                if(mesh_accessor["instanced"])
//                    material.set_shader(ShaderTable::get("inst_default"));
//                else
//                    material.set_shader(ShaderTable::get(material_accessor["shader"]));
//
//                MaterialTable::add(mat_name, std::move(material));
//            }
//
//            MaterialComponent material_component(MaterialTable::get(mat_name));
//            material_component.set_texturing_mode(texturing_mode);
//
//            entity.add_component(std::move(material_component));
//        }
    }

    current_node->add_component(std::move(transform));

	u32 num_children = accessor[model_index].value("child_count", 0);

	for (u32 i = 0; i < num_children; ++i)
	{
		u32 child_index = accessor[model_index]["children"][i];
        current_node->add_child(load_node(accessor, child_index, num_models_checked, scene, renderer));
	}

	++num_models_checked;
	return current_node;
}

void SceneSerializer::load_model(SceneNode* scene_node, const char* model_path, Renderer* renderer)
{
    ModelLoader model_loader(renderer, model_path);
    model_loader.load(scene_node);
}

void SceneSerializer::load_primitive(SceneNode* scene_node, const char* primitive_name, Renderer* renderer)
{
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    switch (str_to_primitive_type(primitive_name))
    {
        case PrimitiveTypes::None:
        {
            break;
        }
        case PrimitiveTypes::Cube:
        {
            vertices = Cube::vertices;
            indices = Cube::indices;
            break;
        }
        case PrimitiveTypes::Quad:
        {
            vertices = Quad::vertices;
            indices = Quad::indices;
            break;
        }
    }

    Transform transform{};
    MeshComponent mesh_component{};
    mesh_component.mesh.vertex_buffer = renderer->create_buffer({
        .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        .size = (u32)(sizeof(vertices[0]) * vertices.size()),
        .data = vertices.data()
    });

    mesh_component.mesh.index_count = indices.size();
    mesh_component.mesh.index_buffer = renderer->create_buffer({
        .usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        .size = (u32)(sizeof(indices[0]) * indices.size()),
        .data = indices.data()
    });

    auto* new_node = new SceneNode();
    new_node->set_name(primitive_name);
    mesh_component.set_mesh_name(primitive_name);
    new_node->add_component(std::move(transform));
    new_node->add_component(std::move(mesh_component));
    scene_node->add_child(new_node);
}