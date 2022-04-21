#include <iostream>
#include <stdio.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

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
	unsigned int vertex_array, vertex_buffer;
	unsigned int a_position = 0, a_colour = 1;

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

	float vertices[] =
	{
		-0.5f, -0.5f, 0.f, 1.f, 0.f, 0.f, 1.f,
		 0.5f, -0.5f, 0.f, 0.f, 1.f, 0.f, 1.f,
		 0.f,   0.5f, 0.f, 0.f, 0.f, 1.f, 1.f
	};

	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(a_position);
	glVertexAttribPointer(a_position, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)0);

	glEnableVertexAttribArray(a_colour);
	glVertexAttribPointer(a_colour, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*) (sizeof(float) * 3));

	while (!glfwWindowShouldClose(window))
	{
		double time = glfwGetTime();
		printf("Time: %f\n", time);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	
	glfwTerminate();
	return 0;
}