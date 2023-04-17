#include "pch.h"
#include "LightManager.h"
#include "Renderer.h"
#include "SceneNode.h"
#include "Entity.h"
#include "Shader.h"
#include "Camera.h"
#include "Buffer.h"
#include "components/Transform.h"
#include "components/Light.h"

#include <glm/vec3.hpp>

// TODO: remove later
#include <glad/glad.h>

enum class PointLightBufferOffsets
{
    colour = 0,
    position = 16,
    range = 28,
    brightness = 32,
    shadow_casting = 36,
    shadow_far_plane = 40,
    shadow_bias = 44,
    total_offset = 48
};

enum class DirectLightBufferOffsets
{
    active = 0,
    colour = 16,
    direction = 32,
    brightness = 44,
    shadow_map = 48,
    total_offset = 64
};

LightManager::LightManager() {}

void LightManager::get_lights(const SceneNode& node)
{
	if (node.entity->has_component<PointLight>())
	{
        m_point_lights.push_back(node.entity);
	}
	else if (node.entity->has_component<DirectionalLight>())
	{
        m_direct_light = node.entity;
	}

	for (const SceneNode& n : node)
	{
		get_lights(n);
	}
}

void LightManager::init_lights()
{
    if(m_point_lights.size() > 0)
        adjust_point_lights_buff();
    else
        ShaderTable::get("default")->set_uniform_1i("u_num_point_lights", 0);

    if(m_direct_light)
    {
        m_direct_light_buffer = std::make_unique<ShaderStorageBuffer>(ShaderStorageBuffer((int)DirectLightBufferOffsets::total_offset));
        m_direct_light_buffer->link(2);

        m_direct_light_buffer->set_data_scalar_i((int)DirectLightBufferOffsets::active, true);
    }
}

void LightManager::update_lights(const std::vector<RenderObject>& render_list, const std::shared_ptr<Camera>& camera)
{
    int index = 0;
	for (std::list<std::shared_ptr<Entity>>::iterator it = m_point_lights.begin(); it != m_point_lights.end(); ++it)
	{
		if (*it && (*it)->has_component<PointLight>())
		{
			auto& transform = (*it)->get_component<Transform>();
			auto& point_light = (*it)->get_component<PointLight>();
			glm::vec3 pos = transform.get_parent_pos() + transform.get_position();

            m_point_light_buffer->set_data_vec4((int)PointLightBufferOffsets::colour + ((int)PointLightBufferOffsets::total_offset * index), point_light.get_colour());
            m_point_light_buffer->set_data_vec3((int)PointLightBufferOffsets::position + ((int)PointLightBufferOffsets::total_offset * index), pos);
            m_point_light_buffer->set_data_scalar_f((int)PointLightBufferOffsets::range + ((int)PointLightBufferOffsets::total_offset * index), point_light.get_range());
            m_point_light_buffer->set_data_scalar_f((int)PointLightBufferOffsets::brightness + ((int)PointLightBufferOffsets::total_offset * index), point_light.get_brightness());
            m_point_light_buffer->set_data_scalar_i((int)PointLightBufferOffsets::shadow_casting + ((int)PointLightBufferOffsets::total_offset * index), point_light.is_casting_shadow());

            // TODO: consolidate into uniform buffer
            ShaderTable::get("default")->set_uniform_3f("u_cam_pos", camera->get_pos());
            ShaderTable::get("inst_default")->set_uniform_3f("u_cam_pos", camera->get_pos());
            ShaderTable::get("pbr_standard")->set_uniform_3f("u_cam_pos", camera->get_pos());
            ShaderTable::get("blinn-phong")->set_uniform_3f("u_cam_pos", camera->get_pos());

            if(point_light.is_casting_shadow())
            {
                if(!(point_light.get_shadow_buffer()))
                    point_light.shadow_init(pos);

                if(point_light.has_shadow_info_changed())
                    point_light.shadow_resize(pos);

                if(transform.has_position_changed())
                    point_light.shadow_update_transforms(pos);


                std::vector<glm::mat4> shadow_transforms = point_light.get_shadow_transforms();
                ShaderTable::get("shadow_cubemap")->set_uniform_mat4f("u_shadow_transforms[0]", shadow_transforms[0]);
                ShaderTable::get("shadow_cubemap")->set_uniform_mat4f("u_shadow_transforms[1]", shadow_transforms[1]);
                ShaderTable::get("shadow_cubemap")->set_uniform_mat4f("u_shadow_transforms[2]", shadow_transforms[2]);
                ShaderTable::get("shadow_cubemap")->set_uniform_mat4f("u_shadow_transforms[3]", shadow_transforms[3]);
                ShaderTable::get("shadow_cubemap")->set_uniform_mat4f("u_shadow_transforms[4]", shadow_transforms[4]);
                ShaderTable::get("shadow_cubemap")->set_uniform_mat4f("u_shadow_transforms[5]", shadow_transforms[5]);

                ShaderTable::get("shadow_cubemap")->set_uniform_3f("u_light_pos", pos);
                ShaderTable::get("shadow_cubemap")->set_uniform_1f("u_far_plane", point_light.get_far_plane());

                m_point_light_buffer->set_data_scalar_f((int)PointLightBufferOffsets::shadow_far_plane + ((int)PointLightBufferOffsets::total_offset * index), point_light.get_far_plane());
                m_point_light_buffer->set_data_scalar_f((int)PointLightBufferOffsets::shadow_bias + ((int)PointLightBufferOffsets::total_offset * index), point_light.get_shadow_bias());
                m_point_shadow_maps->set_data_scalar_u64(index * sizeof(uint64_t), glGetTextureHandleARB(point_light.get_shadow_cubemap()));

                point_light.bind_shadow_map();
                auto [width, height] = point_light.get_shadow_dimensions();
                Renderer::shadow_pass(render_list, width, height, true);
            }
		}

        ++index;
	}

	if (m_direct_light)
	{
        auto& direct_light = m_direct_light->get_component<DirectionalLight>();
        auto& dl_transform = m_direct_light->get_component<Transform>();

        m_direct_light_buffer->set_data_vec4((int)DirectLightBufferOffsets::colour, direct_light.get_colour());
        m_direct_light_buffer->set_data_vec3((int)DirectLightBufferOffsets::direction, direct_light.get_direction());
        m_direct_light_buffer->set_data_scalar_f((int)DirectLightBufferOffsets::brightness, direct_light.get_brightness());

        // TODO: consolidate into uniform buffer
        ShaderTable::get("default")->set_uniform_3f("u_cam_pos", camera->get_pos());
        ShaderTable::get("inst_default")->set_uniform_3f("u_cam_pos", camera->get_pos());
        ShaderTable::get("pbr_standard")->set_uniform_3f("u_cam_pos", camera->get_pos());
        ShaderTable::get("blinn-phong")->set_uniform_3f("u_cam_pos", camera->get_pos());

        if(direct_light.is_casting_shadow())
        {
            if(!(direct_light.get_shadow_buffer()))
            {
                direct_light.shadow_init(dl_transform.get_position());
            }

            ShaderTable::get("shadow_map")->set_uniform_mat4f("u_light_space_view", direct_light.get_light_view());
            ShaderTable::get("shadow_map")->set_uniform_mat4f("u_light_space_projection", direct_light.get_light_projection());
            ShaderTable::get("inst_shadow_map")->set_uniform_mat4f("u_light_space_view", direct_light.get_light_view());
            ShaderTable::get("inst_shadow_map")->set_uniform_mat4f("u_light_space_projection", direct_light.get_light_projection());
            // TODO: uniform buffer
            ShaderTable::get("default")->set_uniform_mat4f("u_light_proj", direct_light.get_light_projection() * direct_light.get_light_view());
            ShaderTable::get("inst_default")->set_uniform_mat4f("u_light_proj", direct_light.get_light_projection() * direct_light.get_light_view());
            direct_light.bind_shadow_map();
            Renderer::shadow_pass(render_list);
            // FIXME
            uint64_t handle = glGetTextureHandleARB(direct_light.get_shadow_map());
            glMakeTextureHandleResidentARB(handle);
            m_direct_light_buffer->set_data_scalar_u64((int)DirectLightBufferOffsets::shadow_map, handle);
        }
    }
}

void LightManager::remove_directional_light()
{
    m_direct_light_buffer->set_data_scalar_i((int)DirectLightBufferOffsets::active, false);
}

void LightManager::add_point_light(const SceneNode& node)
{
    m_point_lights.push_back(node.entity);
    adjust_point_lights_buff();
}

void LightManager::remove_point_light(const SceneNode& node)
{
    for (std::list<std::shared_ptr<Entity>>::iterator it = m_point_lights.begin(); it != m_point_lights.end(); ++it)
    {
        if(node.entity == *it)
        {
            m_point_lights.erase(it);
            break;
        }
    }

    adjust_point_lights_buff();
}

void LightManager::adjust_point_lights_buff()
{
    int num_point_lights = m_point_lights.size();
    int buffer_size = (int)PointLightBufferOffsets::total_offset * num_point_lights;
    m_point_light_buffer = std::make_unique<ShaderStorageBuffer>(ShaderStorageBuffer(buffer_size));
    m_point_light_buffer->link(1);
    ShaderTable::get("default")->set_uniform_1i("u_num_point_lights", num_point_lights);

    m_point_shadow_maps = std::make_unique<ShaderStorageBuffer>(ShaderStorageBuffer(sizeof(uint64_t) * num_point_lights));
    m_point_shadow_maps->link(3);
}