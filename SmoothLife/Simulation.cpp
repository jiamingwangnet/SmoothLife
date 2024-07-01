#include "Simulation.h"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Simulation::Simulation(const std::string& vertexShader, const std::string& fragmentShader, unsigned int resolutionX, unsigned int resolutionY, unsigned int windowWidth, unsigned int windowHeight)
	: vertp{vertexShader}, fragp{fragmentShader}, resX{resolutionX}, resY{resolutionY}, width{windowWidth}, height{windowHeight}
{}

void Simulation::Init()
{
	InitGLFW();
	InitQuad();
	InitRendering();

	shader = Shader{ vertp.c_str(), fragp.c_str() };
}

void Simulation::MainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		// render offscreen
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// draw to input state (thus is reversed)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render0 ? texture1 : texture0, 0);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// draw

		processInput();

		// render onscreen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// set active textures
		// TEXTURE0 = timestamp 0 (input state)
		// TEXTURE1 = timestamp 1 (output state that is rendered)

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, render0 ? texture1 : texture0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, render0 ? texture0 : texture1);

		shader.Use();

		shader.SetInt(INPUT_UNIFORM, 0);
		shader.SetInt(OUTPUT_UNIFORM, 1);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, sizeof(quadIndices), GL_UNSIGNED_INT, 0);

		render0 = !render0;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
}

void Simulation::InitGLFW()
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// TODO: add fullscreen
	window = glfwCreateWindow(width, height, "SmoothLife", nullptr, nullptr);

	if (window == nullptr)
	{
		throw std::runtime_error{ "Could not init GLFW" };
	}

	glViewport(0, 0, width, height);

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); });
}

void Simulation::InitQuad()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Simulation::InitRendering()
{
	// draw to texture by rendering to it

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);


	glGenTextures(1, &texture0);
	glBindTexture(GL_TEXTURE_2D, texture0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resX, resY, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resX, resY, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// user first draws to texture0
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture0, 0);

	// stencil and depth is not needed

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) throw std::runtime_error{ "Framebuffer is not complete" };

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Simulation::processInput()
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

Simulation::Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	std::string vertexSource;
	std::string fragmentSource;
	std::ifstream vertexFile;
	std::ifstream fragmentFile;

	// enable fstream exception throwing
	vertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fragmentFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		vertexFile.open(vertexPath);
		fragmentFile.open(fragmentPath);
		std::stringstream vertexStream, fragmentStream;

		vertexStream << vertexFile.rdbuf();
		fragmentStream << fragmentFile.rdbuf();

		vertexFile.close();
		fragmentFile.close();

		vertexSource = vertexStream.str();
		fragmentSource = fragmentStream.str();
	}
	catch (const std::ifstream::failure& e)
	{
		std::cout << "Cannot read shader files" << std::endl;
	}

	const char* vShaderSource = vertexSource.c_str();
	const char* fShaderSource = fragmentSource.c_str();

	// compile shaders

	unsigned int vertex, fragment;
	int success;
	char infoLog[512];

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderSource, nullptr);
	glCompileShader(vertex);

	// print compile errors
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
		std::cout << "Vertex shader compilation failed: \n" << infoLog << std::endl;
	}

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderSource, nullptr);
	glCompileShader(fragment);

	// print compile errors
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
		std::cout << "Fragment shader compilation failed: \n" << infoLog << std::endl;
	}

	id = glCreateProgram();
	glAttachShader(id, vertex);
	glAttachShader(id, fragment);
	glLinkProgram(id);

	// print link errors
	glGetShaderiv(id, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(id, 512, nullptr, infoLog);
		std::cout << "Shader link failed: \n" << infoLog << std::endl;
	}

	// delete the linked shaders
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Simulation::Shader::Shader::Use()
{
	glUseProgram(id);
}

void Simulation::Shader::Shader::SetBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

void Simulation::Shader::Shader::SetInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Simulation::Shader::Shader::SetFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Simulation::Shader::Shader::SetVec4(const std::string& name, float f0, float f1, float f2, float f3)
{
	glUniform4f(glGetUniformLocation(id, name.c_str()), f0, f1, f2, f3);
}

void Simulation::Shader::Shader::SetVec3(const std::string& name, float f0, float f1, float f2)
{
	glUniform3f(glGetUniformLocation(id, name.c_str()), f0, f1, f2);
}

void Simulation::Shader::Shader::SetMat4(const std::string& name, glm::mat4& mat)
{
	glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}
