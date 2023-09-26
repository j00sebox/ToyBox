#pragma once

#include "scene/Scene.hpp"

class MenuBar
{
public:
    explicit MenuBar(Scene* current_scene) : m_current_scene(current_scene) {}

    void display();

private:
    Scene* m_current_scene;

    void display_scene_dropdown();
    void display_add_dropdown();
    void display_settings_dropdown();
};
