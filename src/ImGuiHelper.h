#pragma once

class ImVec4;
class ImVec2;

// helper functions for making things with ImGui

void coloured_label(const char* label, ImVec4 colour, ImVec2 size);

void texture_viewer(unsigned int texture_id, float texture_width, float texture_height);

void display_empty_texture();