#pragma once
#include "Types.hpp"

class ImVec4;
class ImVec2;

// helper functions for making things with ImGui
namespace imguih
{
    void coloured_label(const char* label, ImVec4 colour, ImVec2 size);
    void texture_viewer(u32 texture_id, f32 texture_width, f32 texture_height);
    void display_empty_texture();
}
