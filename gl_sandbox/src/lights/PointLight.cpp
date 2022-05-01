#include "pch.h"
#include "PointLight.h"

PointLight::PointLight(float radius)
	: m_radius(radius)
{
	m_position = { 0.f, 0.f, 0.f };
	m_colour = { { 1.f, 1.f, 1.f }, 1.f }; // temporary
}

void PointLight::set_postion(const mathz::Vec3 position)
{
	m_position = position;
}

void PointLight::set_colour(const mathz::Vec4 colour)
{
	m_colour = colour;
}
