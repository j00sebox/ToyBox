#include "pch.h"
#include "Engine.hpp"
#include "Input.h"

Engine::Engine(int width, int height) :
    m_running(false)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(width, height, "ToyBox", nullptr, nullptr);

    m_renderer = new Renderer(m_window);

    //FIXME
    Input::m_window_handle = m_window;
}

Engine::~Engine()
{
    delete m_renderer;
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Engine::run()
{
    m_running = true;
    while (!glfwWindowShouldClose(m_window) && m_running)
    {
        glfwPollEvents();

        if (Input::is_key_pressed(GLFW_KEY_ESCAPE))
        {
            m_running = false;
            continue;
        }
    }
}
