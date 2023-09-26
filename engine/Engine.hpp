#pragma once
#include "CommonTypes.hpp"
#include "editor/MenuBar.hpp"
#include "editor/Inspector.hpp"
#include "editor/Diagnostics.hpp"

#include <GLFW/glfw3.h>
#include <TaskScheduler.h>

class Renderer;
class Scene;

class Engine
{
public:
    Engine(i32 width, i32 height);
    ~Engine();

    void load_scene(const char* scene_name);

    void run();

private:
    GLFWwindow* m_window;
    enki::TaskScheduler* m_scheduler;
    Renderer* m_renderer;
    Scene* m_scene;
    MenuBar* m_menu_bar;
    Inspector* m_inspector;
    Diagnostics* m_diagnostics_window;

    bool m_running;
    f32 m_prev_time;

    f32 get_delta_time();
};
