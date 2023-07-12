#pragma once

#include "Component.h"

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

class Transform final : public Component
{
public:
    Transform() : position(glm::vec3()), uniformScale(1.f), rotateAngle(0.f), rotateAxis(glm::vec3(1.f, 0.f, 0.f)),
                  positionMatrix(glm::mat4(1.f)), rotationMatrix(glm::mat4(1.f)), scaleMatrix(glm::mat4(1.f)) {}
	void translate(const glm::vec3& pos);
	void scale(float s);
	void rotate(float angle, const glm::vec3& axis);

	[[nodiscard]] const glm::vec3& get_position() const { return position; }
	[[nodiscard]] const glm::vec3& get_rotate_axis() const { return rotateAxis; }
	[[nodiscard]] float get_rotate_angle() const { return rotateAngle; }
	[[nodiscard]] float get_uniform_scale() const { return uniformScale; }
	[[nodiscard]] glm::mat4 get_transform() const;
    void resolveParentChange(const Transform& oldParent, const Transform& parent);

	[[nodiscard]] const char* get_name() const override { return "Transform"; }
	[[nodiscard]] size_t get_type() const override { return typeid(Transform).hash_code(); }
	void imgui_render() override;
	void serialize(nlohmann::json& accessor) const override;

	Transform operator* (const Transform& other) const;

private:
    void updateTransform();
    void resolveVectors();

    glm::mat4 positionMatrix;
    glm::mat4 rotationMatrix;
    glm::mat4 scaleMatrix;
    glm::vec3 position;
    float uniformScale;
    float rotateAngle;
	glm::vec3 rotateAxis;
};

