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
    position = pos;
    positionMatrix = glm::translate(glm::mat4(1.f), position);
}

void Transform::scale(float s)
{
    uniformScale = s;
    scaleMatrix = glm::scale(glm::mat4(1.f), glm::vec3(uniformScale, uniformScale, uniformScale));
}

void Transform::rotate(float angle, const glm::vec3& axis)
{
    rotateAngle = angle;
    rotateAxis = axis;
    rotationMatrix = glm::rotate(glm::mat4(1.f), glm::radians(rotateAngle), rotateAxis);
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

void Transform::imgui_render()
{
	float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 button_size = { line_height + 3.0f, line_height };

    glm::vec3 prev_pos = position;
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

	ImGui::Text("\nRotation: ");
	ImGui::Text("angle"); ImGui::SameLine();
	ImGui::DragFloat("##angle", &rotateAngle);
	coloured_label("i", ImVec4{ 0.0f, 0.8f, 0.96f, 1.0f }, button_size);
	ImGui::SameLine(0.f, 21.f);
	ImGui::SliderFloat("##i", &rotateAxis.x, -1.f, 1.f);
	coloured_label("j", ImVec4{ 0.84f, 0.78f, 0.1f, 1.0f }, button_size);
	ImGui::SameLine(0.f, 21.f);
	ImGui::SliderFloat("##j", &rotateAxis.y, -1.f, 1.f);
	coloured_label("k", ImVec4{ 0.97f, 0.04f, 1.f, 1.0f }, button_size);
	ImGui::SameLine(0.f, 21.f);
	ImGui::SliderFloat("##k", &rotateAxis.z, -1.f, 1.f);
	glm::normalize(rotateAxis);

	ImGui::Text("\nScale: ");
	ImGui::InputFloat("uniform scale", &uniformScale);

    updateTransform();
}

void Transform::serialize(json& accessor) const
{
	accessor["transform"]["translate"][0] = position.x;
	accessor["transform"]["translate"][1] = position.y;
	accessor["transform"]["translate"][2] = position.z;

	accessor["transform"]["rotation"][0] = rotateAngle;
	accessor["transform"]["rotation"][1] = rotateAxis.x;
	accessor["transform"]["rotation"][2] = rotateAxis.y;
	accessor["transform"]["rotation"][3] = rotateAxis.z;

	accessor["transform"]["scale"] = uniformScale;
}

void Transform::updateTransform()
{
    positionMatrix = glm::translate(glm::mat4(1.f), position);
    scaleMatrix = glm::scale(glm::mat4(1.f), glm::vec3(uniformScale, uniformScale, uniformScale));
    rotationMatrix = glm::rotate(glm::mat4(1.f), glm::radians(rotateAngle), rotateAxis);
}

// helper function to pull the necessary vectors from the transform matrices
void Transform::resolveVectors()
{
    position.x = positionMatrix[3][0];
    position.y = positionMatrix[3][1];
    position.z = positionMatrix[3][2];

    uniformScale = scaleMatrix[0][0];

    glm::quat newQuat = glm::quat_cast(rotationMatrix);
    rotateAngle = glm::degrees(glm::angle(newQuat));
    rotateAxis = glm::axis(newQuat);
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
