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
    m_translate = glm::translate(glm::mat4(1.0f), pos);
}

void Transform::scale(float s)
{
	m_uniform_scale = s;
    m_scale = glm::scale(glm::mat4(1), glm::vec3(s, s, s));
}

void Transform::rotate(float angle, const glm::vec3& axis)
{
	m_rotate_angle = angle;
	m_rotate_axis = axis;

    glm::quat q = glm::angleAxis(glm::radians(angle), axis);
	m_rotation = glm::mat4_cast(q);
}

glm::mat4 Transform::get_transform() const
{
	return  m_rotation * m_scale * m_translate;
}

void Transform::set_parent_offsets(glm::vec3 parent_pos, float parent_scale)
{
	m_parent_position = parent_pos;
	m_parent_scale = parent_scale;
}

void Transform::imgui_render()
{
	glm::vec3 position = m_position;

	float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 button_size = { line_height + 3.0f, line_height };

	ImGui::Text("\nPosition: ");
	coloured_label("x", ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f }, button_size);
	ImGui::SameLine();
	ImGui::DragFloat("##x", &position.x);
	coloured_label("y", ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f }, button_size);
	ImGui::SameLine();
	ImGui::DragFloat("##y", &position.y);
	coloured_label("z", ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f }, button_size);
	ImGui::SameLine();
	ImGui::DragFloat("##z", &position.z);
	translate(position);

	float angle = m_rotate_angle;
	glm::vec3 axis = m_rotate_axis;
	ImGui::Text("\nRotation: ");
	ImGui::Text("angle"); ImGui::SameLine();
	ImGui::DragFloat("##angle", &angle);
	coloured_label("i", ImVec4{ 0.0f, 0.8f, 0.96f, 1.0f }, button_size);
	ImGui::SameLine(0.f, 21.f);
	ImGui::SliderFloat("##i", &axis.x, -1.f, 1.f);
	coloured_label("j", ImVec4{ 0.84f, 0.78f, 0.1f, 1.0f }, button_size);
	ImGui::SameLine(0.f, 21.f);
	ImGui::SliderFloat("##j", &axis.y, -1.f, 1.f);
	coloured_label("k", ImVec4{ 0.97f, 0.04f, 1.f, 1.0f }, button_size);
	ImGui::SameLine(0.f, 21.f);
	ImGui::SliderFloat("##k", &axis.z, -1.f, 1.f);
	glm::normalize(axis);
	rotate(angle, axis);

	float uniform_scale = m_uniform_scale;
	ImGui::Text("\nScale: ");
	ImGui::InputFloat("uniform scale", &uniform_scale);
	scale(uniform_scale);
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
	accessor["transform"]["parent_scale"] = m_parent_scale;
}

Transform Transform::operator*(const Transform& other) const
{
	Transform new_transform{};

	new_transform.translate(m_position + other.m_position);
	new_transform.rotate(m_rotate_angle + other.m_rotate_angle, m_rotate_axis + other.m_rotate_axis);
	new_transform.scale(m_uniform_scale * other.m_uniform_scale);

	return new_transform;
}
