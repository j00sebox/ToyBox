#pragma once

#include <imgui.h>

// helper functions for making things with ImGui

void coloured_label(const char* label, ImVec4 colour, ImVec2 size)
{
	ImGui::PushStyleColor(ImGuiCol_Button, colour);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colour);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, colour);
	ImGui::Button(label, size);
	ImGui::PopStyleColor(3);
}