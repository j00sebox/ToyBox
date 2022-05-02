#pragma once

#include "VertexArray.h"

#include "mathz/Matrix.h"

class SceneObject
{
public:
	virtual void draw() const = 0;

	virtual void translate(const mathz::Vec3& pos);
	virtual void scale(float s);
	virtual void rotate(float angle, const mathz::Vec3& axis);

	virtual void set_name(const std::string& name);
	virtual void set_shader(const std::string& shader_name);
	[[nodiscard]] const std::string& get_name() const { return m_name; }
	[[nodiscard]] const std::string& get_shader() const { return m_shader_name; }
	[[nodiscard]] const mathz::Vec3& get_position() const { return m_postion; }
	[[nodiscard]] const mathz::Vec3& get_rotate_axis() const { return m_rotate_axis; }
	[[nodiscard]] float get_rotate_angle() const { return m_rotate_angle; }
	[[nodiscard]] mathz::Mat4 get_transform() const;

protected:
	std::string m_name;
	std::string m_shader_name;

	mathz::Vec3 m_postion;
	mathz::Vec3 m_rotate_axis;
	float m_rotate_angle = 0.f;

	mathz::Mat4 m_scale;
	mathz::Mat4 m_rotation;
	mathz::Mat4 m_translate;
};

