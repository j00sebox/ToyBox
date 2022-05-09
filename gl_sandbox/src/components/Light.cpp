#include "pch.h"
#include "Light.h"

#include "Shader.h"

#include <imgui.h>
#include <imgui_internal.h>

Light::Light()
{
	m_colour = { 1.f, 1.f, 1.f, 1.f };
}

void Light::parse(json info)
{
	
}

void Light::imgui_render()
{
	ImGui::Text("\nLight\n");

	float colour[4] = {
		m_colour.x,
		m_colour.y,
		m_colour.z,
		m_colour.w
	};

	ImGui::ColorEdit4("colour", colour);

	m_colour.x = colour[0];
	m_colour.y = colour[1];
	m_colour.z = colour[2];
	m_colour.w = colour[3];
}

void PointLight::on_remove()
{
	ShaderLib::get("texture2D")->set_uniform_1i("u_use_pl", 0);
}

void PointLight::parse(json info)
{
}

void PointLight::imgui_render()
{
	Light::imgui_render();

	ImGui::Text("\n");

	ImGui::InputFloat("radius", &m_radius);
	ImGui::InputFloat("range", &m_range);
}
