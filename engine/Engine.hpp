#pragma once
#include "Types.hpp"

class Renderer;
class Scene;

class Engine
{
public:
    Engine(u32 width, u32 height);
    ~Engine();

    void load_scene(const char* scene_name);

    void run();

private:
    GLFWwindow* m_window;
    enki::TaskScheduler* m_scheduler;
    Renderer* m_renderer;
    Scene* m_scene;

    bool m_running;
    f32 m_prev_time;

    f32 get_delta_time();
};
