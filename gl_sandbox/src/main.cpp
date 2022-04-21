#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>
#include <fstream>
#include <malloc.h>

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

std::pair<std::string, std::string> load_shader(const std::string& path)
{
	std::ifstream stream(path);

	std::string line;
	std::stringstream ss[2];

	unsigned int index;

	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
			{
				index = 0;
			}

			if (line.find("fragment") != std::string::npos)
			{
				index = 1;
			}
		}
		else
		{
			ss[index] << line << "\n";
		}
	}

	return { ss[0].str(), ss[1].str() };
}

int main()
{
	unsigned int vertex_array, vertex_buffer, program, basic_shader_v, basic_shader_f;
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

	auto [vertex_shader, fragment_shader] = load_shader("res/shaders/basic.shader");

	const char* vs = vertex_shader.c_str();

	basic_shader_v = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(basic_shader_v, 1, &vs, nullptr);

	glCompileShader(basic_shader_v);

	int success = 0;
	glGetShaderiv(basic_shader_v, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		int log_sz;
		glGetShaderiv(basic_shader_v, GL_INFO_LOG_LENGTH, &log_sz);

		char* info_log = (char*)_malloca(log_sz * sizeof(char));
		glGetShaderInfoLog(basic_shader_v, log_sz, nullptr, info_log);

		std::cout << info_log << "\n";

		glDeleteShader(basic_shader_v);
		__debugbreak();
	}
	else
	{
		std::cout << "Shader compilation successful!\n";
	}

	const char* fs = fragment_shader.c_str();

	basic_shader_f = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(basic_shader_f, 1, &fs, nullptr);

	glCompileShader(basic_shader_f);

	glGetShaderiv(basic_shader_f, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		int log_sz;
		glGetShaderiv(basic_shader_f, GL_INFO_LOG_LENGTH, &log_sz);

		char* info_log = (char*)_malloca(log_sz * sizeof(char));
		glGetShaderInfoLog(basic_shader_f, log_sz, nullptr, info_log);

		std::cout << info_log << "\n";

		glDeleteShader(basic_shader_f);
		__debugbreak();
	}
	else
	{
		std::cout << "Shader compilation successful!\n";
	}

	program = glCreateProgram();

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

	glAttachShader(program, basic_shader_v);
	glAttachShader(program, basic_shader_f);

	glLinkProgram(program);

	glDetachShader(program, basic_shader_v);
	glDetachShader(program, basic_shader_f);

	glUseProgram(program);

	while (!glfwWindowShouldClose(window))
	{
		double time = glfwGetTime();
		//printf("Time: %f\n", time);

		
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	
	glfwTerminate();
	return 0;
}