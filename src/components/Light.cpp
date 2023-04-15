#include "pch.h"
#include "Light.h"
#include "ImGuiHelper.h"
#include "Log.h"

#include <imgui.h>
#include <nlohmann/json.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

// TODO: remove later
#include "GLError.h"
#include <glad/glad.h>

using namespace nlohmann;

Light::Light()
{
	m_colour = { 1.f, 1.f, 1.f, 1.f };
    m_brightness = 1.f;
    m_shadow_casting = false;
}

void Light::imgui_render()
{
	float colour[4] = {
		m_colour.x,
		m_colour.y,
		m_colour.z,
		m_colour.w
	};

	ImGui::ColorEdit4("Colour", colour);

	m_colour.x = colour[0];
	m_colour.y = colour[1];
	m_colour.z = colour[2];
	m_colour.w = colour[3];

	ImGui::SliderFloat("Brightness", &m_brightness, 0.f, 10.f);

    ImGui::Checkbox("Shadow Casting", &m_shadow_casting);
}

void DirectionalLight::imgui_render()
{
	Light::imgui_render();

	ImGui::Text("\n");

	float direction[3] = { m_direction.x, m_direction.y, m_direction.z };

	ImGui::InputFloat3("Direction", direction);

	m_direction.x = direction[0];
	m_direction.y = direction[1]; 
	m_direction.z = direction[2];

    if(m_shadow_casting)
    {
        texture_viewer(m_shadow_map->get_depth_attachment(), 2048, 2048);
    }
}

void DirectionalLight::serialize(json& accessor) const
{
	accessor["light"]["type"] = "directional_light";

	accessor["light"]["colour"][0] = m_colour.x;
	accessor["light"]["colour"][1] = m_colour.y;
	accessor["light"]["colour"][2] = m_colour.z;
	accessor["light"]["colour"][3] = m_colour.w;

	accessor["light"]["brightness"] = m_brightness;

	accessor["light"]["direction"][0] = m_direction.x;
	accessor["light"]["direction"][1] = m_direction.y;
	accessor["light"]["direction"][2] = m_direction.z;

    accessor["light"]["cast_shadow"] = m_shadow_casting;
}

void DirectionalLight::shadow_init(const glm::vec3& light_pos)
{
    m_shadow_map = std::make_shared<FrameBuffer>(SHADOW_WIDTH, SHADOW_HEIGHT, 1);

    m_shadow_map->bind();
    // don't check for completeness here since it will fail due to no colour attachment
    m_shadow_map->attach_texture(AttachmentTypes::Depth);

    info(m_shadow_map->is_complete());

    // set up light matrices
    m_light_projection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 0.1f, 100.f);

    m_light_view = glm::lookAt(light_pos, glm::vec3(0), glm::vec3(0, 1, 0));
}

void PointLight::imgui_render()
{
	Light::imgui_render();

	ImGui::Text("\n");

	ImGui::InputFloat("Range", &m_range);
}

void PointLight::serialize(json& accessor) const
{
	accessor["light"]["type"] = "point_light";

	accessor["light"]["colour"][0] = m_colour.x;
	accessor["light"]["colour"][1] = m_colour.y;
	accessor["light"]["colour"][2] = m_colour.z;
	accessor["light"]["colour"][3] = m_colour.w;

	accessor["light"]["range"] = m_range;
    accessor["light"]["brightness"] = m_brightness;
}

void PointLight::shadow_init(const glm::vec3 &light_pos)
{
    m_shadow_cubemap = std::make_shared<CubeMap>(GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT);

    m_shadow_map = std::make_shared<FrameBuffer>(SHADOW_WIDTH, SHADOW_HEIGHT, 1);
    m_shadow_map->bind();
    m_shadow_map->attach_texture(AttachmentTypes::Depth, m_shadow_cubemap->get_id());

    glm::mat4 shadow_proj = glm::perspective(glm::radians(90.0f), 1.f, 1.f, 100.f);

    m_shadow_transforms.push_back(shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
    m_shadow_transforms.push_back(shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
    m_shadow_transforms.push_back(shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    m_shadow_transforms.push_back(shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0)));
    m_shadow_transforms.push_back(shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0)));
    m_shadow_transforms.push_back(shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0)));
}

