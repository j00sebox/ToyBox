#pragma once

#include "SceneObject.h"
#include "VertexArray.h"

#include "mathz/Vector.h"

class PointLight : public SceneObject
{
public:
	PointLight(float radius = 1.f);

	void draw() const override;
	void set_colour(const mathz::Vec4& colour);

	[[nodiscard]] const mathz::Vec3& get_postion() const { return m_position; }
	[[nodiscard]] const mathz::Vec4& get_colour() const { return m_colour; }
	[[nodiscard]] float get_radius() const { return m_radius; }

private:
	float m_radius;
	mathz::Vec3 m_position;
	mathz::Vec4 m_colour;

	unsigned int m_indices_count;
	VertexArray m_va;
};

