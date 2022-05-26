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

	void glfw_window_resize_callback(GLFWwindow* window, int width, int height)
	{
		EventList::e_resize.execute_function(width, height);
	}
}

Window::Window(int width, int height)
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
	m_frame_buffer->attach_texture(AttachmentTypes::Colour);
	m_frame_buffer->attach_renderbuffer(AttachmentTypes::Depth | AttachmentTypes::Stencil); // create one render buffer object for both

	assert(m_frame_buffer->is_complete());

	m_frame_buffer->unbind();
}

Window::~Window()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_window_handle);
	glfwTerminate();
}

void Window::display_render_context()
{
	ImGui::Begin("##RenderWindow", (bool*)true, ImGuiWindowFlags_NoTitleBar);
	ImVec2 avail_size = ImGui::GetContentRegionAvail();
    ImVec2 pos = ImGui::GetCursorScreenPos();
	
	if(prev_fb_width != avail_size.x || prev_fb_height != avail_size.y)
	{
		info("New screen size [x: {}, y: {}]\n", avail_size.x, avail_size.y);
        resize_frame_buffer((int)avail_size.x, (int)avail_size.y);
		EventList::e_resize.execute_function((int)avail_size.x, (int)avail_size.y);
		prev_fb_width = avail_size.x;
		prev_fb_height = avail_size.y;
	}
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddImage((void*)m_frame_buffer->get_colour_attachment(),
        pos,
        ImVec2(pos.x + avail_size.x, pos.y + avail_size.y),
        ImVec2(0, 1),
        ImVec2(1, 0));
    ImGui::End();
}

void Window::resize_frame_buffer(int width, int height)
{
    m_frame_buffer->unbind();
    m_frame_buffer.reset(new FrameBuffer(width, height));
    m_frame_buffer->bind();

    // want the main buffer to have a texture colour for imgui
    m_frame_buffer->attach_texture(AttachmentTypes::Colour);
    m_frame_buffer->attach_renderbuffer(AttachmentTypes::Depth | AttachmentTypes::Stencil); // create one render buffer object for both

    assert(m_frame_buffer->is_complete());

    m_frame_buffer->unbind();

}

void Window::begin_frame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	m_frame_buffer->bind();
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
	double time = glfwGetTime() * 1000.0;

	auto delta_time = (float)(time - prev_time);

	prev_time = time;

	return delta_time;
}
