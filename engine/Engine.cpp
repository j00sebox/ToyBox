#include "pch.h"
#include "Engine.hpp"
#include "Input.hpp"
#include "Log.hpp"
#include "rendering/Renderer.hpp"
#include "scene/Scene.hpp"
#include "scene/SceneSerializer.hpp"
#include "profiler/Timer.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

Engine::Engine(i32 width, i32 height) :
    m_running(false),
    m_prev_time(0.f)
{
    Timer timer;
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(width, height, "ToyBox", nullptr, nullptr);

    enki::TaskSchedulerConfig scheduler_config;
    scheduler_config.numTaskThreadsToCreate = 4;

    m_scheduler = new enki::TaskScheduler();
    m_scheduler->Initialize(scheduler_config);

    m_renderer = new Renderer(m_window, m_scheduler);
    m_scene = new Scene();
    m_scene->camera->resize(width, height);
    load_scene("../assets/scenes/test.scene");

    m_editor = new Editor(m_scene, m_renderer);

    //FIXME
    Input::m_window_handle = m_window;

    info("Startup Time: {} ms", timer.stop());
}

Engine::~Engine()
{
    delete m_editor;
    m_scene->close(m_renderer);
    delete m_scene;
    delete m_renderer;
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Engine::load_scene(const char* scene_name)
{
    SceneSerializer::open(scene_name, m_scene, m_renderer);
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

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        bool show_dockspace = true;
        ImGui::Begin("DockSpace", &show_dockspace, window_flags);

        // submit the dockspace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("DSP_ID");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        }

        ImGui::PopStyleVar(2);
        ImGui::End();

        ImGui::Begin("Viewport");
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        ImGui::Image(m_renderer->get_current_viewport_image(), ImVec2{viewportPanelSize.x, viewportPanelSize.y});
        ImGui::End();

        m_editor->display();
        ImGui::Render();

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
