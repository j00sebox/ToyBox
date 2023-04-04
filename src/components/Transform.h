#pragma once

#include "Component.h"

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

class Transform final : public Component
{
public:
	void translate(const glm::vec3& pos);
	void scale(float s);
	void rotate(float angle, const glm::vec3& axis);

	[[nodiscard]] const glm::vec3& get_position() const { return m_position; }
	[[nodiscard]] const glm::vec3& get_rotate_axis() const { return m_rotate_axis; }
	[[nodiscard]] float get_rotate_angle() const { return m_rotate_angle; }
	[[nodiscard]] float get_uniform_scale() const { return m_uniform_scale; }
	[[nodiscard]] glm::mat4 get_transform() const;

	// TODO: add solution for rotation
	void set_parent_offsets(glm::vec3 parent_pos, float parent_scale);
	[[nodiscard]] const glm::vec3& get_parent_pos() const { return m_parent_position; }
	[[nodiscard]] float get_parent_scale() const { return m_parent_scale; }

	[[nodiscard]] const char* get_name() const override { return "Transform"; }
	[[nodiscard]] size_t get_type() const override { return typeid(Transform).hash_code(); }
	void imgui_render() override;
	void serialize(nlohmann::json& accessor) const override;

	Transform operator* (const Transform& other) const;

private:
    glm::mat4 m_scale = glm::mat4(1.f);
    glm::mat4 m_rotation = glm::mat4(1.f);;
	glm::mat4 m_translate = glm::mat4(1.f);;

	glm::vec3 m_position;
	glm::vec3 m_rotate_axis;
	float m_rotate_angle = 0.f;
	float m_uniform_scale = 1.f;

	// parent offsets
	glm::vec3 m_parent_position;
	float m_parent_scale = 1.f;
};

