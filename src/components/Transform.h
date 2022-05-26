#pragma once

#include "Component.h"

#include <mathz/Matrix.h>

class Transform final : public Component
{
public:
	void translate(const mathz::Vec3& pos);
	void scale(float s);
	void rotate(float angle, const mathz::Vec3& axis);

	[[nodiscard]] const mathz::Vec3& get_position() const { return m_position; }
	[[nodiscard]] const mathz::Vec3& get_rotate_axis() const { return m_rotate_axis; }
	[[nodiscard]] float get_rotate_angle() const { return m_rotate_angle; }
	[[nodiscard]] float get_uniform_scale() const { return m_uniform_scale; }
	[[nodiscard]] mathz::Mat4 get_transform() const;

	// TODO: add solution for rotation
	void set_parent_offsets(mathz::Vec3 parent_pos, float parent_scale);
	[[nodiscard]] const mathz::Vec3& get_parent_pos() const { return m_parent_position; }
	[[nodiscard]] float get_parent_scale() const { return m_parent_scale; }

	[[nodiscard]] const char* get_name() const override { return "Transform"; }
	[[nodiscard]] size_t get_type() const override { return typeid(Transform).hash_code(); }
	void imgui_render() override;
	void serialize(nlohmann::json& accessor) const override;

	Transform operator* (const Transform& other) const;

private:
	mathz::Mat4 m_scale;
	mathz::Mat4 m_rotation;
	mathz::Mat4 m_translate;

	mathz::Vec3 m_position;
	mathz::Vec3 m_rotate_axis;
	float m_rotate_angle = 0.f;
	float m_uniform_scale = 1.f;

	// parent offsets
	mathz::Vec3 m_parent_position;
	float m_parent_scale = 1.f;
};

