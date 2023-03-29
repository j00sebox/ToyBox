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

#include <spdlog/fmt/bundled/format.h>
#include <glm/vec3.hpp>

enum class PointLightBufferOffsets
{
    active = 0,
    colour = 16,
    position = 32,
    range = 44,
    radius = 48,
    brightness = 52,
    total_offset = 64
};

enum class DirectLightBufferOffsets
{
    active = 128,
    colour = 144,
    direction = 160,
    brightness = 172
};

LightManager::LightManager()
{
    m_light_uniform_buffer = std::make_unique<UniformBuffer>(UniformBuffer(192));
    m_light_uniform_buffer->link(1);

	for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
	{
		m_point_lights[i] = nullptr;
		m_available_point_lights.push(i);
	}
}

void LightManager::set_lights(const SceneNode& node)
{
	if (node.entity->has_component<PointLight>())
	{
		add_point_light(node);
	}
	else if (node.entity->has_component<DirectionalLight>())
	{
        m_direct_light = node.entity.get();
        m_light_uniform_buffer->set_data_scalar_i((int)DirectLightBufferOffsets::active, true);
	}

	for (const SceneNode& n : node)
	{
		set_lights(n);
	}
}

void LightManager::update_lights(const std::vector<RenderObject>& render_list, const std::shared_ptr<Camera>& camera)
{
	for (int i = 0; i < m_point_lights.size(); ++i)
	{
		if (m_point_lights[i] && m_point_lights[i]->has_component<PointLight>())
		{
			auto& transform = m_point_lights[i]->get_component<Transform>();
			auto& point_light = m_point_lights[i]->get_component<PointLight>();
			glm::vec3 pos = transform.get_parent_pos() + transform.get_position();

            m_light_uniform_buffer->set_data_vec4((int)PointLightBufferOffsets::colour + ((int)PointLightBufferOffsets::total_offset * i), point_light.get_colour());
            m_light_uniform_buffer->set_data_vec3((int)PointLightBufferOffsets::position + ((int)PointLightBufferOffsets::total_offset * i), pos);
            m_light_uniform_buffer->set_data_scalar_f((int)PointLightBufferOffsets::range + ((int)PointLightBufferOffsets::total_offset * i), point_light.get_range());
            m_light_uniform_buffer->set_data_scalar_f((int)PointLightBufferOffsets::radius + ((int)PointLightBufferOffsets::total_offset * i), point_light.get_radius());
            m_light_uniform_buffer->set_data_scalar_f((int)PointLightBufferOffsets::brightness + ((int)PointLightBufferOffsets::total_offset * i), point_light.get_brightness());

            //ShaderLib::get("pbr_standard")->set_uniform_4f("u_emissive_colour", point_light.get_colour());
            ShaderLib::get("default")->set_uniform_3f("u_cam_pos", camera->get_pos());
            ShaderLib::get("pbr_standard")->set_uniform_3f("u_cam_pos", camera->get_pos());
            ShaderLib::get("blinn-phong")->set_uniform_3f("u_cam_pos", camera->get_pos());
		}
	}

	if (m_direct_light)
	{
        auto& direct_light = m_direct_light->get_component<DirectionalLight>();
        auto& dl_transform = m_direct_light->get_component<Transform>();

        m_light_uniform_buffer->set_data_vec4((int)DirectLightBufferOffsets::colour, direct_light.get_colour());
        m_light_uniform_buffer->set_data_vec3((int)DirectLightBufferOffsets::direction, glm::vec3(0.f, 20.f, 20.f));
        m_light_uniform_buffer->set_data_scalar_f((int)DirectLightBufferOffsets::brightness, direct_light.get_brightness());

        ShaderLib::get("default")->set_uniform_3f("u_cam_pos", camera->get_pos());
        ShaderLib::get("pbr_standard")->set_uniform_3f("u_cam_pos", camera->get_pos());
        ShaderLib::get("blinn-phong")->set_uniform_3f("u_cam_pos", camera->get_pos());

        if(direct_light.is_casting_shadow())
        {
            ShaderLib::get("shadow_map")->set_uniform_mat4f("u_light_space_view", direct_light.get_light_view());
            ShaderLib::get("shadow_map")->set_uniform_mat4f("u_light_space_projection", direct_light.get_light_projection());
            direct_light.bind_shadow_map();
            Renderer::shadow_pass(render_list);
        }
    }
}

void LightManager::remove_directional_light()
{
    m_direct_light = nullptr;
    m_light_uniform_buffer->set_data_scalar_i((int)DirectLightBufferOffsets::active, false);
}

void LightManager::add_point_light(const SceneNode& node)
{
	int index = m_available_point_lights.front();
	m_point_lights[index] = node.entity.get();
	m_available_point_lights.pop();
    m_light_uniform_buffer->set_data_scalar_i((int)PointLightBufferOffsets::active + ((int)PointLightBufferOffsets::total_offset * index), true);
}

void LightManager::remove_point_light(const SceneNode& node)
{
	for (int i = 0; i < m_point_lights.size(); ++i)
	{
		if (m_point_lights[i] == node.entity.get())
		{
			m_point_lights[i] = nullptr;
			m_available_point_lights.push(i);
            m_light_uniform_buffer->set_data_scalar_i((int)PointLightBufferOffsets::active + ((int)PointLightBufferOffsets::total_offset * i), false);
			break;
		}
	}
}
