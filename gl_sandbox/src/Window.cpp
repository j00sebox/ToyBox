#include "pch.h"
#include "Window.h"

#include "Log.h"
#include "Input.h"
#include "events/EventList.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

extern "C"
{
	void glfw_error_callback(int error, const char* description)
	{
		fprintf(stderr, "[%i] Error: %s\n", error, description);
	}

	void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

Window::Window(int width, int height)
	: m_width(width), m_height(height)
{
	if (!glfwInit())
		ASSERT(false);

	m_window_handle = glfwCreateWindow(width, height, "gl_sandbox", NULL, NULL);

	if (!m_window_handle)
		ASSERT(false);

	glfwMakeContextCurrent(m_window_handle);
	gladLoadGL();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	const char* glsl_version = "#version 410";
	
	// setup backends
	ImGui_ImplGlfw_InitForOpenGL(m_window_handle, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	
	// v-sync on
	glfwSwapInterval(1);

	// setup callbacks
	glfwSetErrorCallback(glfw_error_callback);

	Input::m_window_handle = m_window_handle;
}

Window::~Window()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_window_handle);
	glfwTerminate();
}

void Window::begin_frame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow();

	ImGui::Begin("FPS");

	ImGui::Text("Avg. %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
}

void Window::end_frame()
{
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(m_window_handle);
	glfwPollEvents();
}

float Window::get_delta_time()
{
	float dx = 0.f, dy = 0.f;
	double time = glfwGetTime() * 1000.0;

	auto delta_time = (float)(time - prev_time);

	prev_time = time;

	return delta_time;
}
