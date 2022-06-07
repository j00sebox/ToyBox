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
		m_direct_light = &node.entity->get_component<DirectionalLight>();
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

            // TODO: make uniform buffer for this stuff
			ShaderLib::get("pbr_standard")->set_uniform_4f(fmt::format("point_lights[{}].colour", i), point_light.get_colour());
			ShaderLib::get("pbr_standard")->set_uniform_1f(fmt::format("point_lights[{}].brightness", i), point_light.get_brightness());
			ShaderLib::get("pbr_standard")->set_uniform_3f(fmt::format("point_lights[{}].position", i), pos);
			ShaderLib::get("pbr_standard")->set_uniform_1f(fmt::format("point_lights[{}].radius", i), point_light.get_radius());
			ShaderLib::get("pbr_standard")->set_uniform_1f(fmt::format("point_lights[{}].range", i), point_light.get_range());
			ShaderLib::get("pbr_standard")->set_uniform_3f("u_cam_pos", camera->get_pos());
			ShaderLib::get("pbr_standard")->set_uniform_4f("u_emissive_colour", point_light.get_colour());

            m_point_light_buffer->set_data_vec4((int)PointLightBufferOffsets::colour + ((int)PointLightBufferOffsets::total_offset * i), point_light.get_colour());
            m_point_light_buffer->set_data_vec3((int)PointLightBufferOffsets::position + ((int)PointLightBufferOffsets::total_offset * i), pos);
            m_point_light_buffer->set_data_scalar_f((int)PointLightBufferOffsets::range + ((int)PointLightBufferOffsets::total_offset * i), point_light.get_range());
            m_point_light_buffer->set_data_scalar_f((int)PointLightBufferOffsets::radius + ((int)PointLightBufferOffsets::total_offset * i), point_light.get_radius());
            
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

void LightManager::add_point_light(const SceneNode& node)
{
	int index = m_available_point_lights.front();
	m_point_lights[index] = node.entity.get();
	m_available_point_lights.pop();
	ShaderLib::get("pbr_standard")->set_uniform_1i(fmt::format("point_lights[{}].active", index), true);
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
			ShaderLib::get("pbr_standard")->set_uniform_1i(fmt::format("point_lights[{}].active", i), false);
            m_point_light_buffer->set_data_scalar_i((int)PointLightBufferOffsets::active + ((int)PointLightBufferOffsets::total_offset * i), false);
			break;
		}
	}
}
