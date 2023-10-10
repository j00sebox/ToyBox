#pragma once
#include "MenuBar.hpp"
#include "Inspector.hpp"
#include "Diagnostics.hpp"

class Scene;
class Renderer;

class Editor
{
public:
    explicit Editor(Scene* scene, Renderer* renderer) :
        m_scene(scene),
        m_renderer(renderer) {}
    void display();


private:
    Scene* m_scene;
    Renderer* m_renderer;

    MenuBar m_menu_bar{};
    Inspector m_inspector{};
    Diagnostics m_diagnostics_window{};
};

