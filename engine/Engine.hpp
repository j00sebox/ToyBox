#pragma once

#include "rendering/Renderer.hpp"

class Engine
{
public:
    Engine(u32 width, u32 height);
    ~Engine();

    void run();

private:
    GLFWwindow* m_window;
    enki::TaskScheduler* m_scheduler;
    Renderer* m_renderer;
    Scene* m_scene;

    bool m_running;
};
