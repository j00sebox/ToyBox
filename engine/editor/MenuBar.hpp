#pragma once
#include "scene/Scene.hpp"

struct MenuBar
{
    void display(Scene* scene, Renderer* renderer);

private:
    static void display_scene_dropdown(const Scene* scene);
    void display_add_dropdown(Scene* scene, Renderer* renderer);
    void display_settings_dropdown();

    bool name_exists(const std::string& name, const SceneNode* scene_node);
    void set_unique_name(SceneNode* scene_node, const SceneNode* root);
};
