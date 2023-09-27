#pragma once
#include "scene/Scene.hpp"

struct MenuBar
{
    void display(Scene* scene, Renderer* renderer);

private:
    static void display_scene_dropdown(const Scene* scene);
    static void display_add_dropdown(Scene* scene, Renderer* renderer);
    void display_settings_dropdown();
};
