#pragma once

#include "IComponent.h"

#include "mathz/Matrix.h"

class Transform : public IComponent
{
public:
	void translate(const mathz::Vec3& pos);
	void scale(float s);
	void rotate(float angle, const mathz::Vec3& axis);

	[[nodiscard]] const mathz::Vec3& get_position() const { return m_postion; }
	[[nodiscard]] const mathz::Vec3& get_rotate_axis() const { return m_rotate_axis; }
	[[nodiscard]] float get_rotate_angle() const { return m_rotate_angle; }
	[[nodiscard]] mathz::Mat4 get_transform() const;

	void parse(json info) override;
	void imgui_render() override;

private:
	mathz::Vec3 m_postion;
	mathz::Vec3 m_rotate_axis;
	float m_rotate_angle = 0.f;

	mathz::Mat4 m_scale;
	mathz::Mat4 m_rotation;
	mathz::Mat4 m_translate;
};

