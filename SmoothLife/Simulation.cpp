#include "Simulation.h"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

Simulation::Simulation(const std::string& vertexShader, const std::string& simVertShader, const std::string& fragmentShader, const std::string& passthroughFrag, const std::string& brushFrag, unsigned int resolutionX, unsigned int resolutionY, unsigned int windowWidth, unsigned int windowHeight)
	: gui{uniforms, color}, vertp{vertexShader}, fragp{fragmentShader}, brushp{brushFrag}, simvp{simVertShader}, passp{passthroughFrag}, resX{resolutionX}, resY{resolutionY}, width{windowWidth}, height{windowHeight}
{}

void Simulation::Init()
{
	InitGLFW();
	InitQuad();
	InitRendering();

	shader = Shader{ simvp.c_str(), fragp.c_str() };
	passthrough = Shader{ vertp.c_str(), passp.c_str() };
	brush = Shader{ vertp.c_str(), brushp.c_str() };

	gui.SetWindow(window);
	gui.Init();

	// generate UBO and bind

	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Uniforms), &uniforms, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

	unsigned int bufferIdx = glGetUniformBlockIndex(shader.GetId(), "SimData");
	glUniformBlockBinding(shader.GetId(), bufferIdx, 0);
}

void Simulation::MainLoop()
{
	glBindVertexArray(vao);

	// bind texture0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture0);

	// bind texutre1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	glDisable(GL_DEPTH_TEST);

	/*
		Render user input to texture0
			V
		Use user input to render next timestep to texture1
			V
		Render next timestep (texture1) back to texture0
			V
		Render texture1 to screen
	*/

	float invResX = 1.0f / float(resX);
	float invResY = 1.0f / float(resY);

	shader.Use();
	shader.SetInt(INPUT_UNIFORM, 0);
	shader.SetVec2("resolution", (float)resX, (float)resY);
	shader.SetVec2("invResolution", (float)invResX, (float)invResY);

	passthrough.Use();
	passthrough.SetInt(INPUT_UNIFORM, 1);

	while (!glfwWindowShouldClose(window))
	{
		gui.RenderStart();
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// render offscreen
		// render to first fbo
		glBindFramebuffer(GL_FRAMEBUFFER, fbo); // rendering to the same framebuffer creates artifacts

		// drawing code here (render circles to the screen, etc):
		// ...
		processInput();
		// ...

		 //glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// render to second framebuffer with next timestep
		glActiveTexture(GL_TEXTURE0);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo2);

		shader.Use();
		shader.SetVec3("color", color.x, color.y, color.z );

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// store the calculated timestep to texture0
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glActiveTexture(GL_TEXTURE1);

		passthrough.Use();

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// render onscreen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		gui.CreateGui();

		// render the texture on to the screen with a passthrough (new timestep)
		// render texture1 (next timestep) to screen

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		gui.RenderEnd();
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

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error{ "Failed to initialize GLAD" };
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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resX, resY, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// user draws to texture0
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture0, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) throw std::runtime_error{ "Framebuffer is not complete" };

	// Output to fbo2
	glGenFramebuffers(1, &fbo2);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo2);

	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resX, resY, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture1, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) throw std::runtime_error{ "Framebuffer 2 is not complete" };

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Simulation::processInput()
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !(gui.GetIO()->WantCaptureMouse))
	//if (true)
	{
		double x,y;

		glfwGetCursorPos(window, &x, &y);

		//std::cout << x << ' ' << y << std::endl;

		DrawPixels(x,y);
		//DrawPixels(width/2.0,height/2.0);
	}
}

void Simulation::DrawPixels(double x, double y)
{
	// test code

	brush.Use();
	brush.SetInt(INPUT_UNIFORM, 0);
	brush.SetVec2("xy", x / (double)width, y/(double)height);
	brush.SetVec2("resolution", (float)resX, (float)resY);
	brush.SetFloat("depth", 1.0f);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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
		std::cout << "Cannot read shader files " << e.what() << std::endl;
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

unsigned int Simulation::Shader::GetId() const
{
	return id;
}

void Simulation::Shader::Shader::Use() const
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

void Simulation::Shader::Shader::SetVec4(const std::string& name, float f0, float f1, float f2, float f3) const
{
	glUniform4f(glGetUniformLocation(id, name.c_str()), f0, f1, f2, f3);
}

void Simulation::Shader::Shader::SetVec3(const std::string& name, float f0, float f1, float f2) const
{
	glUniform3f(glGetUniformLocation(id, name.c_str()), f0, f1, f2);
}

void Simulation::Shader::Shader::SetVec2(const std::string& name, float f0, float f1) const
{
	glUniform2f(glGetUniformLocation(id, name.c_str()), f0, f1);
}

void Simulation::Shader::Shader::SetMat4(const std::string& name, glm::mat4& mat)
{
	glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

Simulation::GUIHandler::GUIHandler(GLFWwindow* window, Uniforms& uniforms, glm::vec3& color)
	: window{window}, uniforms{uniforms}, color{color}
{}

Simulation::GUIHandler::GUIHandler(Uniforms & uniforms, glm::vec3& color)
	: uniforms{uniforms}, window{nullptr}, color{color}
{}

void Simulation::GUIHandler::Init()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	io = &ImGui::GetIO();
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

void Simulation::GUIHandler::RenderStart()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::GetIO().IniFilename = nullptr;
	ImGui::GetIO().LogFilename = nullptr;
}

void Simulation::GUIHandler::CreateGui()
{
	ImGui::Begin("Properties");
	ImGui::SetWindowSize({ 350, 600 });

	if (ImGui::InputFloat("ri", &uniforms.ri, 0.001, 0.1))
	{
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Uniforms, ri), sizeof(float), &uniforms.ri);
	}
	if (ImGui::InputFloat("ra", &uniforms.ra, 0.001, 0.1))
	{
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Uniforms, ra), sizeof(float), &uniforms.ra);
	}
	if (ImGui::InputFloat("dt", &uniforms.dt, 0.001, 0.1))
	{
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Uniforms, dt), sizeof(float), &uniforms.dt);
	}
	if (ImGui::InputFloat("alpha_n", &uniforms.alpha_n, 0.001, 0.1))
	{
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Uniforms, alpha_n), sizeof(float), &uniforms.alpha_n);
	}
	if (ImGui::InputFloat("alpha_m", &uniforms.alpha_m, 0.001, 0.1))
	{
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Uniforms, alpha_m), sizeof(float), &uniforms.alpha_m);
	}
	if (ImGui::InputFloat("b1", &uniforms.b1, 0.001, 0.1))
	{
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Uniforms, b1), sizeof(float), &uniforms.b1);
	}
	if (ImGui::InputFloat("b2", &uniforms.b2, 0.001, 0.1))
	{
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Uniforms, b2), sizeof(float), &uniforms.b2);
	}
	if (ImGui::InputFloat("d1", &uniforms.d1, 0.001, 0.1))
	{
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Uniforms, d1), sizeof(float), &uniforms.d1);
	}
	if (ImGui::InputFloat("d2", &uniforms.d2, 0.001, 0.1))
	{
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Uniforms, d2), sizeof(float), &uniforms.d2);
	}
	ImGui::NewLine();

	ImGui::ColorPicker3("Color", &(color.x));
	ImGui::End();
}

void Simulation::GUIHandler::RenderEnd()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Simulation::GUIHandler::Shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
