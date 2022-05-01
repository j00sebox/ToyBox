#pragma once

#include "mathz/Vector.h"

class PointLight
{
public:
	PointLight(float radius);

	void set_postion(const mathz::Vec3 position);
	void set_colour(const mathz::Vec4 colour);

	[[nodiscard]] const mathz::Vec3& get_postion() const { return m_position; }
	[[nodiscard]] const mathz::Vec4& get_colour() const { return m_colour; }
private:
	float m_radius;
	mathz::Vec3 m_position;
	mathz::Vec4 m_colour;
};

