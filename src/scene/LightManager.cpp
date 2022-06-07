#include "pch.h"
#include "LightManager.h"

#include "SceneNode.h"
#include "Entity.h"
#include "Shader.h"
#include "Camera.h"
#include "Buffer.h"
#include "components/Transform.h"
#include "components/Light.h"

#include <spdlog/fmt/bundled/format.h>

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

LightManager::LightManager()
{
    m_point_light_buffer = std::make_unique<UniformBuffer>(UniformBuffer(128));
    m_point_light_buffer->link(1);

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
        set_directional_light(node.entity->get_component<DirectionalLight>());
	}

	for (const SceneNode& n : node)
	{
		set_lights(n);
	}
}

void LightManager::update_lights(const std::shared_ptr<Camera>& camera)
{
	for (int i = 0; i < m_point_lights.size(); ++i)
	{
		if (m_point_lights[i] && m_point_lights[i]->has_component<PointLight>())
		{
			auto& transform = m_point_lights[i]->get_component<Transform>();
			auto& point_light = m_point_lights[i]->get_component<PointLight>();
			mathz::Vec3 pos = transform.get_parent_pos() + transform.get_position();

            m_point_light_buffer->set_data_vec4((int)PointLightBufferOffsets::colour + ((int)PointLightBufferOffsets::total_offset * i), point_light.get_colour());
            m_point_light_buffer->set_data_vec3((int)PointLightBufferOffsets::position + ((int)PointLightBufferOffsets::total_offset * i), pos);
            m_point_light_buffer->set_data_scalar_f((int)PointLightBufferOffsets::range + ((int)PointLightBufferOffsets::total_offset * i), point_light.get_range());
            m_point_light_buffer->set_data_scalar_f((int)PointLightBufferOffsets::radius + ((int)PointLightBufferOffsets::total_offset * i), point_light.get_radius());
            m_point_light_buffer->set_data_scalar_f((int)PointLightBufferOffsets::brightness + ((int)PointLightBufferOffsets::total_offset * i), point_light.get_brightness());

            ShaderLib::get("pbr_standard")->set_uniform_4f("u_emissive_colour", point_light.get_colour());
            ShaderLib::get("pbr_standard")->set_uniform_3f("u_cam_pos", camera->get_pos());
            ShaderLib::get("blinn-phong")->set_uniform_3f("u_cam_pos", camera->get_pos());
		}
	}

	if (m_direct_light)
	{
		ShaderLib::get("pbr_standard")->set_uniform_4f("directional_light.colour", m_direct_light->get_colour());
		ShaderLib::get("pbr_standard")->set_uniform_1f("directional_light.brightness", m_direct_light->get_brightness());
		ShaderLib::get("pbr_standard")->set_uniform_3f("directional_light.direction", m_direct_light->get_direction());
		ShaderLib::get("pbr_standard")->set_uniform_3f("u_cam_pos", camera->get_pos());
		ShaderLib::get("pbr_standard")->set_uniform_4f("u_emissive_colour", m_direct_light->get_colour());

        ShaderLib::get("blinn-phong")->set_uniform_4f("directional_light.colour", m_direct_light->get_colour());
        ShaderLib::get("blinn-phong")->set_uniform_1f("directional_light.brightness", m_direct_light->get_brightness());
        ShaderLib::get("blinn-phong")->set_uniform_3f("directional_light.direction", m_direct_light->get_direction());
        ShaderLib::get("blinn-phong")->set_uniform_3f("u_cam_pos", camera->get_pos());
        ShaderLib::get("blinn-phong")->set_uniform_4f("u_emissive_colour", m_direct_light->get_colour());
	}
}

void LightManager::set_directional_light(const DirectionalLight& dl)
{
    m_direct_light = std::make_shared<DirectionalLight>(dl);
    ShaderLib::get("pbr_standard")->set_uniform_1i("directional_light.active", true);
    ShaderLib::get("blinn-phong")->set_uniform_1i("directional_light.active", true);
}

void LightManager::remove_directional_light()
{
    m_direct_light.reset();
    ShaderLib::get("pbr_standard")->set_uniform_1i("directional_light.active", false);
    ShaderLib::get("blinn-phong")->set_uniform_1i("directional_light.active", false);
}

void LightManager::add_point_light(const SceneNode& node)
{
	int index = m_available_point_lights.front();
	m_point_lights[index] = node.entity.get();
	m_available_point_lights.pop();
    m_point_light_buffer->set_data_scalar_i((int)PointLightBufferOffsets::active + ((int)PointLightBufferOffsets::total_offset * index), true);
}

void LightManager::remove_point_light(const SceneNode& node)
{
	for (int i = 0; i < m_point_lights.size(); ++i)
	{
		if (m_point_lights[i] == node.entity.get())
		{
			m_point_lights[i] = nullptr;
			m_available_point_lights.push(i);
            m_point_light_buffer->set_data_scalar_i((int)PointLightBufferOffsets::active + ((int)PointLightBufferOffsets::total_offset * i), false);
			break;
		}
	}
}
