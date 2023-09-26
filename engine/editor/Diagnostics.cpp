#include "pch.h"
#include "Diagnostics.hpp"

#include <imgui.h>

void Diagnostics::display()
{
    ImGui::Begin("Diagnostics");
   // display_fps();
    ImGui::End();
}

void Diagnostics::display_fps()
{
    ImGui::BeginPopup("FPS");
    ImGui::Text("Avg. %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::EndPopup();
}