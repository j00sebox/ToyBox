#pragma once

#include "Component.h"

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

class Transform final : public Component
{
public:
    Transform() :
            m_dirty(false),
            m_position(glm::vec3()),
            m_uniform_scale(1.f),
            m_rotate_angle(0.f),
            m_rotate_axis(glm::vec3(1.f, 0.f, 0.f)),
            m_transform_matrix(glm::mat4(1.f)),
            m_position_matrix(glm::mat4(1.f)),
            m_rotation_matrix(glm::mat4(1.f)),
            m_scale_matrix(glm::mat4(1.f))
    {}
	void translate(const glm::vec3& pos);
	void scale(float s);
	void rotate(float angle, const glm::vec3& axis);
    void recalculate_transform();

    [[nodiscard]] bool is_dirty() const { return m_dirty; }
	[[nodiscard]] const glm::vec3& get_position() const { return m_position; }
	[[nodiscard]] const glm::vec3& get_rotate_axis() const { return m_rotate_axis; }
	[[nodiscard]] float get_rotate_angle() const { return m_rotate_angle; }
	[[nodiscard]] float get_uniform_scale() const { return m_uniform_scale; }
	[[nodiscard]] const glm::mat4& get_transform() const;
    void resolve_parent_change(const Transform& oldParent, const Transform& parent);

	[[nodiscard]] const char* get_name() const override { return "Transform"; }
	[[nodiscard]] size_t get_type() const override { return typeid(Transform).hash_code(); }
	void imgui_render() override;
	void serialize(nlohmann::json& accessor) const override;

	Transform operator* (const Transform& other) const;

private:
    void update_matrices();
    void resolve_vectors();

    bool m_dirty;
    glm::vec3 m_position;
    float m_uniform_scale;
    float m_rotate_angle;
	glm::vec3 m_rotate_axis;
    glm::mat4 m_transform_matrix;
    glm::mat4 m_position_matrix;
    glm::mat4 m_rotation_matrix;
    glm::mat4 m_scale_matrix;
};

