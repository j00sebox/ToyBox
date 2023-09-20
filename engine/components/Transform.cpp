#include "pch.h"
#include "Transform.h"
#include "util/ImGuiHelper.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <nlohmann/json.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace nlohmann;

void Transform::translate(const glm::vec3& pos)
{
    m_position = pos;
    m_position_matrix = glm::translate(glm::mat4(1.f), m_position);
    m_dirty = true;
}

void Transform::scale(float s)
{
    m_uniform_scale = s;
    m_scale_matrix = glm::scale(glm::mat4(1.f), glm::vec3(m_uniform_scale, m_uniform_scale, m_uniform_scale));
    m_dirty = true;
}

void Transform::rotate(float angle, const glm::vec3& axis)
{
    m_rotate_angle = angle;
    m_rotate_axis = axis;
    m_rotation_matrix = glm::rotate(glm::mat4(1.f), glm::radians(m_rotate_angle), m_rotate_axis);
    m_dirty = true;
}

void Transform::recalculate_transform()
{
    m_transform_matrix = m_position_matrix * m_scale_matrix * m_rotation_matrix;
    m_dirty = false;
}

const glm::mat4& Transform::get_transform() const
{
	return m_transform_matrix;
}

void Transform::resolve_parent_change(const Transform& oldParent, const Transform& parent)
{
    m_position_matrix = oldParent.m_position_matrix * m_position_matrix;
    m_position_matrix = glm::inverse(parent.m_position_matrix) * m_position_matrix;

    m_scale_matrix = oldParent.m_scale_matrix * m_scale_matrix;
    m_scale_matrix = glm::inverse(parent.m_scale_matrix) * m_scale_matrix;

    m_rotation_matrix = oldParent.m_rotation_matrix * m_rotation_matrix;
    m_rotation_matrix = glm::inverse(parent.m_rotation_matrix) * m_rotation_matrix;

    resolve_vectors();
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

    float prev_angle = m_rotate_angle;
    glm::vec3 prev_axis = m_rotate_axis;
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

    float prev_scale = m_uniform_scale;
	ImGui::Text("\nScale: ");
	ImGui::InputFloat("uniform scale", &m_uniform_scale);

    if(!m_dirty)
        m_dirty = ( (prev_pos != m_position) || (prev_angle != m_rotate_angle) || (prev_axis != m_rotate_axis) || (prev_scale != m_uniform_scale) );
    update_matrices();
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
}

void Transform::update_matrices()
{
    m_position_matrix = glm::translate(glm::mat4(1.f), m_position);
    m_scale_matrix = glm::scale(glm::mat4(1.f), glm::vec3(m_uniform_scale, m_uniform_scale, m_uniform_scale));
    m_rotation_matrix = glm::rotate(glm::mat4(1.f), glm::radians(m_rotate_angle), m_rotate_axis);
}

// helper function to pull the necessary vectors from the transform matrices
void Transform::resolve_vectors()
{
    m_position.x = m_position_matrix[3][0];
    m_position.y = m_position_matrix[3][1];
    m_position.z = m_position_matrix[3][2];

    m_uniform_scale = m_scale_matrix[0][0];

    glm::quat newQuat = glm::quat_cast(m_rotation_matrix);
    m_rotate_angle = glm::degrees(glm::angle(newQuat));
    m_rotate_axis = glm::axis(newQuat);
}

Transform Transform::operator*(const Transform& other) const
{
	Transform new_transform{};
    new_transform.m_position_matrix = m_position_matrix * other.m_position_matrix;
    new_transform.m_scale_matrix = m_scale_matrix * other.m_scale_matrix;
    new_transform.m_rotation_matrix = m_rotation_matrix * other.m_rotation_matrix;

    new_transform.resolve_vectors();
    new_transform.recalculate_transform();

	return new_transform;
}
