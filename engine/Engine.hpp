#pragma once
#include "CommonTypes.hpp"
#include "editor/Editor.hpp"

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
    Editor* m_editor;

    bool m_running;
    f32 m_prev_time;

    f32 get_delta_time();
};
