#include "pch.h"
#include "Window.h"

#include "Log.h"
#include "Input.h"
#include "events/EventList.h"

#include "FrameBuffer.h"

#include <imgui.h>
#include <imgui_internal.h>
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

	void glfw_window_resize_callback(GLFWwindow* window, int width, int height)
	{
		EventList::e_resize.execute_function(width, height);
	}
}

Window::Window(int width = 0, int height = 0)
	: m_width(width), m_height(height)
{
	if (!glfwInit())
		ASSERT(false);

	m_window_handle = glfwCreateWindow(width, height, "Toy Box", NULL, NULL);

	if (!m_window_handle)
		ASSERT(false);

	glfwMakeContextCurrent(m_window_handle);
	gladLoadGL();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.WantCaptureMouse = false;
	ImGui::StyleColorsDark();

	const char* glsl_version = "#version 410";
	
	// setup backends
	ImGui_ImplGlfw_InitForOpenGL(m_window_handle, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	
	// v-sync on
	glfwSwapInterval(1);

	// setup callbacks
	glfwSetErrorCallback(glfw_error_callback);
	glfwSetWindowSizeCallback(m_window_handle, glfw_window_resize_callback);

	Input::m_window_handle = m_window_handle;

	m_frame_buffer = std::make_unique<FrameBuffer>();
	m_frame_buffer->bind();

	// want the main buffer to have a texture colour for imgui
	m_frame_buffer->attach_texture(AttachmentType::Colour);
	m_frame_buffer->attach_renderbuffer(AttachmentType::Depth);
	//m_frame_buffer->attach_renderbuffer(AttachmentType::Stencil);

	printf("%i\n", m_frame_buffer->get_colour_attachment());
	printf("%i\n", m_frame_buffer->get_depth_attachment());
	printf("%i\n", m_frame_buffer->get_stencil_attachment());

	assert(m_frame_buffer->is_complete());
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
	m_frame_buffer->bind();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("FPS");

	ImGui::Text("Avg. %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();

	ImGui::Begin("Game Window");
    ImVec2 pos = ImGui::GetCursorScreenPos();
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddImage((void*)m_frame_buffer->get_colour_attachment(),
        pos,
        ImVec2(pos.x + 800, pos.y + 600),
        ImVec2(0, 1),
        ImVec2(1, 0));
    ImGui::End();
}

void Window::end_frame()
{

	m_frame_buffer->unbind();

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
