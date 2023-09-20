#include "pch.h"
#include "Light.h"
#include "util/ImGuiHelper.hpp"
#include "Log.hpp"

#include <imgui.h>
#include <nlohmann/json.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

// TODO: remove later
#include <glad/glad.h>

using namespace nlohmann;

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

    if(m_shadow_casting && m_shadow_map)
    {
        texture_viewer(m_shadow_map->get_depth_attachment(), 2048, 2048);

        ImGui::SliderFloat("Bias", &m_shadow_bias, 0.f, 0.5f);
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
    m_shadow_map = std::make_shared<FrameBuffer>(m_shadow_width, m_shadow_height, 1);

    m_shadow_map->bind();
    m_shadow_map->attach_texture(AttachmentTypes::Depth);

    // set up light matrices
    m_light_projection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 0.1f, 100.f);

    m_light_view = glm::lookAt(light_pos, glm::vec3(0), glm::vec3(0, 1, 0));
}

void PointLight::imgui_render()
{
	Light::imgui_render();

	ImGui::Text("\n");

	ImGui::InputFloat("Range", &m_range);

    if(m_shadow_casting)
    {
        int dimensions = (int)m_shadow_width;
        ImGui::InputInt("Shadow Dimensions", &dimensions);
        ImGui::Text("\n");
        m_shadow_info_change = (dimensions != (int)m_shadow_width);
        m_shadow_width = (unsigned)dimensions; m_shadow_height = (unsigned)dimensions;

        float prev_near = m_shadow_near, prev_far = m_shadow_far;
        ImGui::InputFloat("Near Plane", &m_shadow_near);
        ImGui::InputFloat("Far Plane", &m_shadow_far);
        m_shadow_info_change = m_shadow_info_change || (prev_near != m_shadow_near || prev_far != m_shadow_far);

        ImGui::SliderFloat("Bias", &m_shadow_bias, 0.f, 0.5f);
    }
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
    m_shadow_map = std::make_shared<FrameBuffer>(m_shadow_width, m_shadow_height, 1);
    m_shadow_map->bind();
    m_shadow_map->attach_texture(AttachmentTypes::Depth, CubeMap(GL_DEPTH_COMPONENT, m_shadow_width, m_shadow_height));

    m_shadow_proj = glm::perspective(glm::radians(90.0f), (float)m_shadow_width / (float)m_shadow_height, m_shadow_near, m_shadow_far);

    m_shadow_transforms.push_back(m_shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
    m_shadow_transforms.push_back(m_shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
    m_shadow_transforms.push_back(m_shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    m_shadow_transforms.push_back(m_shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0)));
    m_shadow_transforms.push_back(m_shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0)));
    m_shadow_transforms.push_back(m_shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0)));
}

void PointLight::shadow_resize(const glm::vec3& light_pos)
{
    m_shadow_map = std::make_shared<FrameBuffer>(m_shadow_width, m_shadow_height, 1);
    m_shadow_map->bind();
    m_shadow_map->attach_texture(AttachmentTypes::Depth, CubeMap(GL_DEPTH_COMPONENT, m_shadow_width, m_shadow_height));

    m_shadow_proj = glm::perspective(glm::radians(90.0f), (float)m_shadow_width / (float)m_shadow_height, m_shadow_near, m_shadow_far);

    shadow_update_transforms(light_pos);
}

void PointLight::shadow_update_transforms(const glm::vec3& light_pos)
{
    m_shadow_transforms[0] = m_shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0));
    m_shadow_transforms[1] = m_shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0));
    m_shadow_transforms[2] = m_shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
    m_shadow_transforms[3] = m_shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0));
    m_shadow_transforms[4] = m_shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0));
    m_shadow_transforms[5] = m_shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0));
}



