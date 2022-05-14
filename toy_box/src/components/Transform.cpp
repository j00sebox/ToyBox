#include "pch.h"
#include "Transform.h"

#include "mathz/Quaternion.h"
#include "mathz/Misc.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <json/json.hpp>

using namespace nlohmann;

void Transform::translate(const mathz::Vec3& pos)
{
	m_position = pos;
	m_translate[3][0] = pos.x;	m_translate[3][1] = pos.y;	m_translate[3][2] = pos.z;
}

void Transform::scale(float s)
{
	m_uniform_scale = s;
	m_scale[0][0] = s; m_scale[1][1] = s; m_scale[2][2] = s;
}

void Transform::rotate(float angle, const mathz::Vec3& axis)
{
	m_rotate_angle = angle;
	m_rotate_axis = axis;
	mathz::Quaternion q(mathz::radians(angle), axis);
	m_rotation = q.convert_to_mat();
}

mathz::Mat4 Transform::get_transform() const
{
	return  m_rotation * m_scale * m_translate;
}

void Transform::imgui_render()
{
	mathz::Vec3 position = m_position;

	ImGui::Text("\nPosition: ");
	ImGui::Text("x"); ImGui::SameLine();
	ImGui::DragFloat("##x", &position.x);
	ImGui::Text("y"); ImGui::SameLine();
	ImGui::DragFloat("##y", &position.y);
	ImGui::Text("z"); ImGui::SameLine();
	ImGui::DragFloat("##z", &position.z);
	translate(position);

	float angle = m_rotate_angle;
	mathz::Vec3 axis = m_rotate_axis;
	ImGui::Text("\nRotation: ");
	ImGui::Text("angle"); ImGui::SameLine();
	ImGui::DragFloat("##angle", &angle);
	ImGui::Text("i"); ImGui::SameLine();
	ImGui::SliderFloat("##i", &axis.x, -1.f, 1.f);
	ImGui::Text("j"); ImGui::SameLine();
	ImGui::SliderFloat("##j", &axis.y, -1.f, 1.f);
	ImGui::Text("k"); ImGui::SameLine();
	ImGui::SliderFloat("##k", &axis.z, -1.f, 1.f);
	axis.normalize();
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
}
