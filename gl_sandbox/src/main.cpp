#include "pch.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "Shader.h"
#include "Buffer.h"
#include "VertexArray.h"

#include "GLError.h"

#include <stb_image.h>

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

int main()
{
	unsigned int a_position = 0, a_tex_coord = 1;//a_colour = 1;
	unsigned int texutre;
	int tex_width, tex_height, bpp;

	if (!glfwInit())
		__debugbreak();

	GLFWwindow* window = glfwCreateWindow(640, 480, "gl_sandbox", NULL, NULL);

	if (!window)
		__debugbreak();

	glfwMakeContextCurrent(window);
	gladLoadGL();

	// v-sync on
	glfwSwapInterval(1);
	
	// setup callbacks
	glfwSetErrorCallback(glfw_error_callback);
	glfwSetKeyCallback(window, glfw_key_callback);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	stbi_set_flip_vertically_on_load(1);
	unsigned char* local_buffer = stbi_load("res/textures/enderpearl.png", &tex_width, &tex_height, &bpp, 4);

	// render scope
	{
		glGenTextures(1, &texutre);
		glBindTexture(GL_TEXTURE_2D, texutre);
		
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, local_buffer));
		
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
		basic_shader.bind();

		GL_CALL(glActiveTexture(GL_TEXTURE0));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, texutre));

		while (!glfwWindowShouldClose(window))
		{
			double time = glfwGetTime();

			GL_CALL(glDrawElements(GL_TRIANGLES, ib.get_count(), GL_UNSIGNED_INT, nullptr));

			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}
	
	glfwDestroyWindow(window);
	
	glfwTerminate();
	return 0;
}