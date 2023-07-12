#include "pch.h"
#include "Transform.h"
#include "ImGuiHelper.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <nlohmann/json.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace nlohmann;

void Transform::translate(const glm::vec3& pos)
{
	m_position = pos;
	m_position_changed = true;
    updateTransform();
    positionMatrix = glm::translate(glm::mat4(1.f), m_position);
}

void Transform::scale(float s)
{
	m_uniform_scale = s;
    scaleMatrix = glm::scale(glm::mat4(1.f), glm::vec3(m_uniform_scale, m_uniform_scale, m_uniform_scale));
}

void Transform::rotate(float angle, const glm::vec3& axis)
{
	m_rotate_angle = angle;
	m_rotate_axis = axis;
    rotationMatrix = glm::rotate(glm::mat4(1.f), glm::radians(m_rotate_angle), m_rotate_axis);
}

glm::mat4 Transform::get_transform() const
{
	return positionMatrix * scaleMatrix * rotationMatrix;
}

void Transform::resolveParentChange(const Transform& oldParent, const Transform& newParent)
{
    positionMatrix = oldParent.positionMatrix * positionMatrix;
    positionMatrix = glm::inverse(newParent.positionMatrix) * positionMatrix;

    scaleMatrix = oldParent.scaleMatrix * scaleMatrix;
    scaleMatrix = glm::inverse(newParent.scaleMatrix) * scaleMatrix;

    rotationMatrix = oldParent.rotationMatrix * rotationMatrix;
    rotationMatrix = glm::inverse(newParent.rotationMatrix) * rotationMatrix;

    resolveVectors();
}

void Transform::setTransform(const glm::mat4& _transform)
{
    transform = _transform;
}

void Transform::setTransforms(const glm::mat4& posMat, const glm::mat4& scaleMat, const glm::mat4& rotMat)
{
    positionMatrix = posMat;
    m_position.x = positionMatrix[3][0];
    m_position.y = positionMatrix[3][1];
    m_position.z = positionMatrix[3][2];

}

void Transform::set_parent_offsets(glm::vec3 parent_pos, float parent_scale)
{
	m_parent_position = parent_pos;
	m_parent_scale = parent_scale;
}

void Transform::imgui_render()
{
	float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 button_size = { line_height + 3.0f, line_height };

    glm::vec3 prev_pos = m_position;
	ImGui::Text("\nPosition: ");
	coloured_label("x", ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f }, button_size);
	ImGui::SameLine();
	ImGui::DragFloat("##x", &m_position.x);
	coloured_label("y", ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f }, button_size);
	ImGui::SameLine();
	ImGui::DragFloat("##y", &m_position.y);
	coloured_label("z", ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f }, button_size);
	ImGui::SameLine();
	ImGui::DragFloat("##z", &m_position.z);
    m_position_changed = (m_position != prev_pos);

	ImGui::Text("\nRotation: ");
	ImGui::Text("angle"); ImGui::SameLine();
	ImGui::DragFloat("##angle", &m_rotate_angle);
	coloured_label("i", ImVec4{ 0.0f, 0.8f, 0.96f, 1.0f }, button_size);
	ImGui::SameLine(0.f, 21.f);
	ImGui::SliderFloat("##i", &m_rotate_axis.x, -1.f, 1.f);
	coloured_label("j", ImVec4{ 0.84f, 0.78f, 0.1f, 1.0f }, button_size);
	ImGui::SameLine(0.f, 21.f);
	ImGui::SliderFloat("##j", &m_rotate_axis.y, -1.f, 1.f);
	coloured_label("k", ImVec4{ 0.97f, 0.04f, 1.f, 1.0f }, button_size);
	ImGui::SameLine(0.f, 21.f);
	ImGui::SliderFloat("##k", &m_rotate_axis.z, -1.f, 1.f);
	glm::normalize(m_rotate_axis);

	ImGui::Text("\nScale: ");
	ImGui::InputFloat("uniform scale", &m_uniform_scale);

    updateTransform();
}

void Transform::serialize(json& accessor) const
{
	accessor["transform"]["translate"][0] = m_position.x;
	accessor["transform"]["translate"][1] = m_position.y;
	accessor["transform"]["translate"][2] = m_position.z;

	accessor["transform"]["rotation"][0] = m_rotate_angle;
	accessor["transform"]["rotation"][1] = m_rotate_axis.x;
	accessor["transform"]["rotation"][2] = m_rotate_axis.y;
	accessor["transform"]["rotation"][3] = m_rotate_axis.z;

	accessor["transform"]["scale"] = m_uniform_scale;

	accessor["transform"]["parent_position"][0] = m_parent_position.x;
	accessor["transform"]["parent_position"][1] = m_parent_position.y;
	accessor["transform"]["parent_position"][2] = m_parent_position.z;
	accessor["transform"]["parent_scale"]       = m_parent_scale;

}

void Transform::updateTransform()
{
    positionMatrix = glm::translate(glm::mat4(1.f), m_position);
    scaleMatrix = glm::scale(glm::mat4(1.f), glm::vec3(m_uniform_scale, m_uniform_scale, m_uniform_scale));
    rotationMatrix = glm::rotate(glm::mat4(1.f), glm::radians(m_rotate_angle), m_rotate_axis);
}

// helper function to pull the necessary vectors from the transform matrices
void Transform::resolveVectors()
{
    m_position.x = positionMatrix[3][0];
    m_position.y = positionMatrix[3][1];
    m_position.z = positionMatrix[3][2];

    m_uniform_scale = scaleMatrix[0][0];

    glm::quat newQuat = glm::quat_cast(rotationMatrix);
    m_rotate_angle = glm::degrees(glm::angle(newQuat));
    m_rotate_axis = glm::axis(newQuat);
}

Transform Transform::operator*(const Transform& other) const
{
	Transform new_transform{};
    new_transform.positionMatrix = positionMatrix * other.positionMatrix;
    new_transform.scaleMatrix = scaleMatrix * other.scaleMatrix;
    new_transform.rotationMatrix = rotationMatrix * other.rotationMatrix;

    new_transform.resolveVectors();

	return new_transform;
}
