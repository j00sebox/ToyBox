#include "pch.h"
#include "Editor.hpp"
#include "scene/Scene.hpp"
#include "rendering/Renderer.hpp"

void Editor::display()
{
    m_menu_bar.display(m_scene, m_renderer);
    m_inspector.display(m_scene);
    m_diagnostics_window.display();
}