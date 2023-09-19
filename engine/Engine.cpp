#include "pch.h"
#include "Engine.hpp"
#include "Input.h"

#include "util/ModelLoader.hpp"

Engine::Engine(u32 width, u32 height) :
    m_running(false)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(width, height, "ToyBox", nullptr, nullptr);

    enki::TaskSchedulerConfig scheduler_config;
    scheduler_config.numTaskThreadsToCreate = 4;

    m_scheduler = new enki::TaskScheduler();
    m_scheduler->Initialize(scheduler_config);

    m_renderer = new Renderer(m_window, m_scheduler);
    m_scene = new Scene();
    m_scene->camera.resize(width, height);

    ModelLoader loader(m_renderer, "../assets/models/bunny/scene.gltf");

    Model loaded_model = loader.load();
    glm::mat4 transform = glm::translate(glm::mat4(1), glm::vec3(0.f, 0.f, -2.0f));
    loaded_model.transform = transform;
    m_scene->add_model(loaded_model);

    //FIXME
    Input::m_window_handle = m_window;
}

Engine::~Engine()
{
    m_scene->close(m_renderer);
    delete m_scene;
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

        m_scene->update(get_delta_time());
        m_renderer->render(m_scene);
    }

    m_renderer->wait_for_device_idle();
}

f32 Engine::get_delta_time()
{
    f64 time = glfwGetTime() * 1000.0;
    auto delta_time = (f32)(time - m_prev_time);
    m_prev_time = (f32)time;
    return delta_time;
}
