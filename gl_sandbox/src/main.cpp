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

	while (!glfwWindowShouldClose(window))
	{
		double time = glfwGetTime();
		printf("Time: %f\n", time);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	
	glfwTerminate();
	return 0;
}