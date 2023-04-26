#include "pch.h"
#include "Window.h"
#include "Log.h"
#include "Input.h"
#include "ViewPort.h"
#include "EventList.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glad/glad.h>

extern "C"
{
	void glfw_error_callback(int error, const char* description)
	{
		fprintf(stderr, "[%i] Error: %s\n", error, description);
	}

	void glfw_window_resize_callback(GLFWwindow* window, int width, int height)
	{
		EventList::e_resize.execute_function(width, height);
	}
}

Window::Window(int width, int height, int viewport_width, int viewport_height)
	: m_width(width), m_height(height)
{
	if (!glfwInit())
		fatal("Could not initialize GLFW!");

	m_window_handle = glfwCreateWindow(width, height, "Toy Box", nullptr, nullptr);

	if (!m_window_handle)
		fatal("Window handle is null!");

	glfwMakeContextCurrent(m_window_handle);
	gladLoadGL();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.WantCaptureMouse = false;
	ImGui::StyleColorsDark();

	const char* glsl_version = "#version 420";
	
	// setup backends
	ImGui_ImplGlfw_InitForOpenGL(m_window_handle, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	
	// v-sync on
	glfwSwapInterval(m_vsync);

	// setup callbacks
	glfwSetErrorCallback(glfw_error_callback);
	glfwSetWindowSizeCallback(m_window_handle, glfw_window_resize_callback);

	Input::m_window_handle = m_window_handle;

    m_main_viewport = std::make_unique<ViewPort>(ViewPort(viewport_width, viewport_height, 1));
}

Window::~Window()
{
	m_main_viewport.reset();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_window_handle);
	glfwTerminate();
}

void Window::display_render_context()
{
	ImGui::Begin("##RenderWindow", (bool*)true, ImGuiWindowFlags_NoTitleBar);
	
	m_main_viewport->display();

    ImGui::End();
}

void Window::resize_viewport(int width, int height)
{
    m_main_viewport->resize(width, height, m_main_viewport->get_sample_amount());
}

void Window::bind_viewport() const
{
    m_main_viewport->bind_framebuffer();
}

void Window::change_sample_amount(int new_sample_amount)
{
    auto [vp_width, vp_height] = m_main_viewport->get_dimensions();
    m_main_viewport->resize(vp_width, vp_height, new_sample_amount);
}

void Window::begin_frame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	m_main_viewport->begin_frame();
}

void Window::end_frame()
{
    m_main_viewport->end_frame();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(m_window_handle);
	glfwPollEvents();
}

float Window::get_delta_time()
{
	double time = glfwGetTime() * 1000.0;

	auto delta_time = (float)(time - prev_time);

	prev_time = time;

	return delta_time;
}

void Window::toggle_vsync()
{
    m_vsync = !m_vsync;
    glfwSwapInterval(m_vsync);
}
