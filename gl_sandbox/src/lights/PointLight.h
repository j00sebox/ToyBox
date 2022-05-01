#pragma once

#include "mathz/Vector.h"

#include "Model.h"

class PointLight
{
public:
	PointLight(float radius = 1.f);

	void draw() const;

	void set_postion(const mathz::Vec3 position);
	void set_colour(const mathz::Vec4 colour);

	void translate(const mathz::Vec3& pos)
	{
		m_position = pos;
		m_translate[3][0] = pos.x;	m_translate[3][1] = pos.y;	m_translate[3][2] = pos.z;
	}

	const mathz::Mat4& get_translate() const { return m_translate; }

	[[nodiscard]] const mathz::Vec3& get_postion() const { return m_position; }
	[[nodiscard]] const mathz::Vec4& get_colour() const { return m_colour; }

	std::shared_ptr<ShaderProgram> get_shader() const { return m_shader; }
private:
	float m_radius;
	mathz::Vec3 m_position;
	mathz::Vec4 m_colour;

	unsigned int m_indices_count;
	mathz::Mat4 m_translate;

	VertexArray m_va;
	std::shared_ptr<ShaderProgram> m_shader;
};

