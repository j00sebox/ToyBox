#include "pch.h"
#include "Engine.hpp"

Engine::Engine(int width, int height)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(width, height, "Vulkan Triangle Engine", nullptr, nullptr);

    m_renderer = new Renderer(m_window);
}

Engine::~Engine()
{

}

void Engine::run()
{

}
