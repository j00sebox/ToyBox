#include "pch.h"
#include "Window.h"

#include "Shader.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Texture.h"

#include "GLError.h"

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
		__debugbreak();

	m_window_handle = glfwCreateWindow(width, height, "gl_sandbox", NULL, NULL);

	if (!m_window_handle)
		__debugbreak();

	glfwMakeContextCurrent(m_window_handle);
	gladLoadGL();

	// v-sync on
	glfwSwapInterval(1);

	// setup callbacks
	glfwSetErrorCallback(glfw_error_callback);
	glfwSetKeyCallback(m_window_handle, glfw_key_callback);

	glViewport(0, 0, width, height);

	main_loop();
}

Window::~Window()
{
	glfwDestroyWindow(m_window_handle);
	glfwTerminate();
}

void Window::main_loop()
{
	unsigned int a_position = 0, a_tex_coord = 1;

	Texture2D lava_texture("res/textures/lava.png");
	
	float vertices[] =
	{
		-0.5f, -0.5f, 0.f, 0.f, 0.f,	// 1.f, 0.f, 0.f, 1.f,
			0.5f,  0.5f, 0.f, 1.f, 1.f,  	// 0.f, 0.f, 1.f, 1.f,
		-0.5f,  0.5f, 0.f, 0.f, 1.f,	// 0.f, 1.f, 0.f, 1.f,
			0.5f, -0.5f, 0.f, 1.f, 0.f		// 0.f, 1.f, 0.f, 1.f,
	};

	unsigned int indices[] = {
		0, 1, 2,
		0, 3, 1
	};

	VertexArray va;

	VertexBuffer vb;
	vb.add_data(vertices, sizeof(vertices));

	BufferLayout layout = {
		{a_position, 3, GL_FLOAT, false},
		{a_tex_coord, 2, GL_FLOAT, false}
		//{a_colour, 4, GL_FLOAT, false}
	};

	va.set_layout(vb, layout);

	IndexBuffer ib(indices, sizeof(indices));

	ShaderProgram basic_shader(
		Shader("res/shaders/texture2D/texture2D_vertex.shader", ShaderType::Vertex),
		Shader("res/shaders/texture2D/texture2D_fragment.shader", ShaderType::Fragment)
	);

	va.bind();
	ib.bind();
	lava_texture.bind(0);
	basic_shader.bind();

	while (!glfwWindowShouldClose(m_window_handle))
	{
		double time = glfwGetTime();

		GL_CALL(glDrawElements(GL_TRIANGLES, ib.get_count(), GL_UNSIGNED_INT, nullptr));

		glfwSwapBuffers(m_window_handle);
		glfwPollEvents();
	}
}
