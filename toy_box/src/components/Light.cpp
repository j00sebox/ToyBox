#include "pch.h"
#include "Light.h"

#include "Shader.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <json/json.hpp>

using namespace nlohmann;

Light::Light()
{
	m_colour = { 1.f, 1.f, 1.f, 1.f };
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
}

DirectionalLight::DirectionalLight()
{
	ShaderLib::get("pbr_standard")->set_uniform_1i("directional_light.active", true);
}

void DirectionalLight::on_remove()
{
	ShaderLib::get("pbr_standard")->set_uniform_1i("directional_light.active", false);
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

	m_direction.normalize();
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
}

PointLight::PointLight()
{
	//ShaderLib::get("pbr_standard")->set_uniform_1i(std::format("point_lights[{}].active", m_index), true);
}

void PointLight::on_remove()
{
	//ShaderLib::get("pbr_standard")->set_uniform_1i(std::format("point_lights[{}].active", m_index), false);
}

void PointLight::imgui_render()
{
	Light::imgui_render();

	ImGui::Text("\n");

	ImGui::InputFloat("radius", &m_radius);
	ImGui::InputFloat("range", &m_range);
}

void PointLight::serialize(json& accessor) const
{
	accessor["light"]["type"] = "point_light";

	accessor["light"]["colour"][0] = m_colour.x;
	accessor["light"]["colour"][1] = m_colour.y;
	accessor["light"]["colour"][2] = m_colour.z;
	accessor["light"]["colour"][3] = m_colour.w;

	accessor["light"]["brightness"] = m_brightness;

	accessor["light"]["radius"] = m_radius;
	accessor["light"]["range"] = m_range;
}

