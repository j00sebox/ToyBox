#pragma once

#include "rendering/Renderer.hpp"

class Engine
{
public:
    Engine(int width, int height);
    ~Engine();

    void run();

private:
    GLFWwindow* m_window;
    Renderer* m_renderer;

    bool m_running;
};
