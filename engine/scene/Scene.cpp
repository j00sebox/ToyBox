#include "Scene.hpp"
// #include "StaticRenderer.h"
#include "SceneSerializer.hpp"
//#include "Mesh.h"
//#include "Log.hpp"
#include "components/Transform.h"
// #include "components/Light.h"
#include "components/MeshComponent.h"
#include "rendering/Renderer.hpp"
#include "events/EventList.h"
//#include "ModelLoader.h"

#include <imgui_internal.h>
#include <spdlog/fmt/bundled/format.h>

Scene::Scene()
{
	camera = std::make_shared<Camera>();
}

Scene::~Scene()
{
    //ShaderTable::release();
    // MeshTable::release();
    // MaterialTable::release();
}

void Scene::load(const char* scene)
{
    compile_shaders();
	//SceneSerializer::open(scene, *this, camera, m_skybox, root);

//    for (auto const& [mesh_name, instance_matrices] : instanced_meshes)
//    {
//        MeshTable::get(mesh_name)->make_instanced((int)instance_matrices.size(), instance_matrices);
//    }
}

void Scene::close(Renderer* renderer)
{
    if(skybox)
    {
        renderer->destroy_descriptor_set_layout(skybox->descriptor_set_layout);
        renderer->destroy_pipeline(skybox->pipeline);
        renderer->destroy_texture(skybox->cubemap);
        renderer->destroy_buffer(skybox->vertex_buffer);
        renderer->destroy_buffer(skybox->index_buffer);
    }
    for (auto* scene_node : root)
    {
        delete_node(renderer, scene_node);
    }
}

void Scene::delete_node(Renderer* renderer, SceneNode* scene_node)
{
    if (scene_node->has_component<MeshComponent>())
    {
        auto& mesh_component = scene_node->get_component<MeshComponent>();

        renderer->destroy_buffer(mesh_component.mesh.vertex_buffer);
        renderer->destroy_buffer(mesh_component.mesh.index_buffer);

        for(auto& texture : mesh_component.material.textures)
        {
            renderer->destroy_texture(texture);
        }
    }

    for (auto* child_node : scene_node->m_children)
    {
        delete_node(renderer, child_node);
    }

    delete scene_node;
}

void Scene::save(const std::string& path)
{
	// SceneSerializer::save(path.c_str(), *this, camera, m_skybox, root);
}

void Scene::init()
{
	// EventList::eResize.bind(this, &Scene::window_resize);

    // auto [width, height] = m_window_handle->get_dimensions();
	// m_camera->resize(width, height);
//    m_transforms_buffer = std::make_unique<Buffer>(128, BufferType::UNIFORM);
//    m_transforms_buffer->link(0);
//    m_transforms_buffer->set_data(0, m_camera->camera_look_at());
//    m_transforms_buffer->set_data(64, m_camera->get_perspective());

//	for (const auto& node : *root)
//	{
//		m_light_manager.get_lights(node);
//	}
//
//    m_light_manager.init_lights();
}

void Scene::update(float elapsed_time)
{
//#ifdef DEBUG
//    //Timer timer{};
//#endif
//	StaticRenderer::clear();
    m_render_list.clear();
    mesh_used.clear();

    while (!m_nodes_to_remove.empty())
    {
        remove_node(m_nodes_to_remove.front());
        m_nodes_to_remove.pop();
    }

    // if the camera moved at all we need to adjust the view uniform
    camera->update(elapsed_time);
     //   m_transforms_buffer->set_data(0, m_camera->camera_look_at());

//	if (m_skybox)
//	{
//		//StaticRenderer::draw_skybox(*m_skybox);
//	}

    for (auto const& [mesh_name, instance_matrices] : instanced_meshes)
    {
        //MeshTable::get(mesh_name)->update_instances(instance_matrices);
    }

	for (auto* scene_node : root)
	{
		update_node(scene_node, glm::mat4(1.f));
	}

//    m_light_manager.update_lights(m_render_list, m_camera);
//
//    m_window_handle->bind_viewport();
//    StaticRenderer::render_pass(m_render_list);

}

//void Scene::add_primitive(const char* name)
//{
//	std::string lookup{ name };
//	int i = 1;
//	while (root->exists(lookup))
//	{
//		lookup = std::string(name) + fmt::format(" ({})", i);
//		++i;
//	}
//
//    ModelLoader model_loader(str_to_primitive_type(name));
//
//	Entity entity;
//    entity.set_name(lookup);
//
//    Transform transform;
//
//    if(!MeshTable::exists(name))
//    {
//        Mesh mesh;
//        model_loader.load_mesh(mesh);
//
//        MeshTable::add(name, std::move(mesh));
//    }
//
//    MeshComponent mesh_component;
//    mesh_component.set_mesh(MeshTable::get(name));
//    mesh_component.set_mesh_info(name, "primitive");
//
//    Material material;
//
//    material.set_colour({1.f, 1.f, 1.f, 1.f});
//    material.set_metallic_property(0.f);
//    material.set_roughness(0.f);
//
//    std::string textures[] = {
//            "../assets/textures/white_on_white.jpeg",
//            "none",
//            "none",
//            "none"
//    };
//
//    material.load(textures);
//
//    if(MeshTable::get(name)->is_instanced())
//    {
//        instanced_meshes[name].push_back(transform.get_transform());
//        material.set_shader(ShaderTable::get("inst_default"));
//        MeshTable::get(name)->make_instanced((int)instanced_meshes[name].size(), instanced_meshes[name]);
//        mesh_component.m_instance_id = (int)instanced_meshes[name].size() - 1;
//    }
//    else
//    {
//        material.set_shader(ShaderTable::get("default"));
//    }
//
//    MaterialTable::add(lookup, std::move(material));
//
//    MaterialComponent material_component(MaterialTable::get(lookup));
//    material_component.set_texturing_mode(TexturingMode::NO_TEXTURE);
//
//    entity.add_component(std::move(transform));
//    entity.add_component(std::move(mesh_component));
//	entity.add_component(std::move(material_component));
//
//    root->add_child(SceneNode{std::make_shared<Entity>(std::move(entity))});
//}
//
//void Scene::add_model(const char* name)
//{
//    ModelLoader model_loader(name);
//
//    std::string lookup{ model_loader.get_name() };
//
//    if(root->exists(lookup))
//    {
//        int i = 1;
//        while (root->exists(lookup))
//        {
//            lookup = std::string(lookup) + fmt::format(" ({})", i);
//            ++i;
//        }
//    }
//
//    Entity entity;
//    entity.set_name(lookup);
//
//    Transform transform;
//
//    if(!MeshTable::exists(name))
//    {
//        Mesh mesh;
//        model_loader.load_mesh(mesh);
//
//        MeshTable::add(name, std::move(mesh));
//    }
//
//    MeshComponent meshComponent;
//    meshComponent.set_mesh(MeshTable::get(name));
//    meshComponent.set_mesh_info(name, "gltf");
//
//    Material material;
//
//    material.set_colour({1.f, 1.f, 1.f, 1.f});
//    material.set_metallic_property(0.f);
//    material.set_roughness(0.f);
//
//
//    if(MeshTable::get(name)->is_instanced())
//    {
//        instanced_meshes[name].push_back(transform.get_transform());
//        material.set_shader(ShaderTable::get("inst_default"));
//        MeshTable::get(name)->make_instanced((int)instanced_meshes[name].size(), instanced_meshes[name]);
//        meshComponent.m_instance_id = (int)instanced_meshes[name].size() - 1;
//    }
//    else
//    {
//        material.set_shader(ShaderTable::get("default"));
//    }
//
//    if (!MaterialTable::exists(name))
//    {
//        model_loader.load_material(material);
//        MaterialTable::add(name, std::move(material));
//    }
//
//    MaterialComponent materialComponent(MaterialTable::get(name));
//    materialComponent.set_texturing_mode(TexturingMode::MODEL_DEFAULT);
//
//    entity.add_component(std::move(transform));
//    entity.add_component(std::move(meshComponent));
//    entity.add_component(std::move(materialComponent));
//
//    root->add_child(SceneNode{std::make_shared<Entity>(std::move(entity))});
//}
//
//void Scene::window_resize(int width, int height)
//{
//	m_camera->resize(width, height);
//	StaticRenderer::set_viewport(width, height);
//    m_transforms_buffer->set_data(0, m_camera->camera_look_at());
//    m_transforms_buffer->set_data(64, m_camera->get_perspective());
//}

void Scene::remove_node(SceneNode* node)
{
//	if (node->entity()->has_component<PointLight>())
//	{
//		m_light_manager.remove_point_light(node);
//	}
//    else if(node->entity()->has_component<DirectionalLight>())
//    {
//        m_light_manager.remove_directional_light();
//    }

	if (!root.remove(node))
	{
		fatal("Node not apart of current scene tree!");
	}
	else
	{
        selectedNode = nullptr;
	}
}

void Scene::update_node(SceneNode* scene_node, const glm::mat4& parent_transform)
{
    auto& transform = scene_node->get_component<Transform>();

    if(transform.is_dirty())
    {
        transform.recalculate_transform();
    }

	glm::mat4 relative_transform = parent_transform * transform.get_transform();
    // relative_transform.set_transform(parent_transform.get_transform() * transform.get_transform());

	if (scene_node->has_component<MeshComponent>())
	{
		// auto& material_component = scene_node->entity()->get_component<MaterialComponent>();
		auto& mesh_component = scene_node->get_component<MeshComponent>();

        m_render_list.emplace_back(relative_transform, mesh_component.mesh, mesh_component.material);

//        if(mesh_component.get_mesh()->is_instanced())
//        {
//            std::string mesh_name = MeshTable::find(mesh_component.get_mesh());
//            instanced_meshes[mesh_name][mesh_component.m_instance_id] = relative_transform.get_transform();
//            if(!mesh_used[mesh_name])
//            {
//                m_render_list.emplace_back(RenderObject{RenderCommand::InstancedElementDraw, relative_transform, mesh_component, material_component, (unsigned int)instanced_meshes[mesh_name].size()});
//                mesh_used[mesh_name] = true;
//            }
//
//            if (selectedNode && (scene_node == selectedNode))
//            {
//                m_render_list.emplace_back(RenderObject{RenderCommand::Stencil, relative_transform, mesh_component, material_component});
//            }
//        }
//		else
//		{
//            if (selectedNode && (scene_node == selectedNode))
//            {
//                m_render_list.emplace_back(RenderObject{RenderCommand::Stencil, relative_transform, mesh_component, material_component});
//            }
//            else
//            {
//                m_render_list.emplace_back(RenderObject{RenderCommand::ElementDraw, relative_transform, mesh_component, material_component});
//            }
//		}
	}

	for (auto* node : scene_node->m_children)
	{
		update_node(node, relative_transform);
	}
}

void Scene::set_background_colour(glm::vec4 colour)
{
	m_clear_colour = colour;
	//StaticRenderer::set_clear_colour(colour);
}

//// load the standard shaders
//void Scene::compile_shaders()
//{
//    ShaderTable::add("default", ShaderProgram(
//            Shader("../assets/shaders/default/default_vertex.shader", ShaderType::Vertex),
//            //Shader("../resources/shaders/default/default_geometry.shader", ShaderType::Geometry),
//            Shader("../assets/shaders/default/default_fragment.shader", ShaderType::Fragment)
//    ), true);
//
//    ShaderTable::add("inst_default", ShaderProgram(
//            Shader("../assets/shaders/default/instanced_vertex.shader", ShaderType::Vertex),
//            Shader("../assets/shaders/default/default_fragment.shader", ShaderType::Fragment)
//    ));
//
//    ShaderTable::add("flat_colour", ShaderProgram(
//            Shader("../assets/shaders/flat_colour/flat_colour_vertex.shader", ShaderType::Vertex),
//            Shader("../assets/shaders/flat_colour/flat_colour_fragment.shader", ShaderType::Fragment)
//    ));
//
//    ShaderTable::add("pbr_standard", ShaderProgram(
//            Shader("../assets/shaders/pbr/pbr_standard_vertex.shader", ShaderType::Vertex),
//            Shader("../assets/shaders/pbr/pbr_standard_fragment.shader", ShaderType::Fragment)
//    ));
//
//    ShaderTable::add("blinn-phong", ShaderProgram(
//            Shader("../assets/shaders/blinn-phong/blinn-phong_vertex.shader", ShaderType::Vertex),
//            Shader("../assets/shaders/blinn-phong/blinn-phong_fragment.shader", ShaderType::Fragment)
//    ));
//
//    ShaderTable::add("skybox", ShaderProgram(
//            Shader("../assets/shaders/skybox/skybox_vertex.shader", ShaderType::Vertex),
//            Shader("../assets/shaders/skybox/skybox_fragment.shader", ShaderType::Fragment)
//    ));
//
//    ShaderTable::add("shadow_map", ShaderProgram(
//            Shader("../assets/shaders/shadow_map/shadow_map_vertex.shader", ShaderType::Vertex),
//            Shader("../assets/shaders/shadow_map/shadow_map_fragment.shader", ShaderType::Fragment)
//    ));
//
//    ShaderTable::add("inst_shadow_map", ShaderProgram(
//            Shader("../assets/shaders/shadow_map/instanced_sm_vertex.shader", ShaderType::Vertex),
//            Shader("../assets/shaders/shadow_map/shadow_map_fragment.shader", ShaderType::Fragment)
//    ));
//
//    ShaderTable::add("shadow_cubemap", ShaderProgram(
//            Shader("../assets/shaders/shadow_map/shadow_cubemap_vertex.shader", ShaderType::Vertex),
//            Shader("../assets/shaders/shadow_map/shadow_cubemap_geometry.shader", ShaderType::Geometry),
//            Shader("../assets/shaders/shadow_map/shadow_cubemap_fragment.shader", ShaderType::Fragment)
//    ));
//}
//
//void Scene::recompile_shaders()
//{
//    ShaderTable::recompile();
//}


