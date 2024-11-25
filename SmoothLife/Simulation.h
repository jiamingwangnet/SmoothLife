#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"

/*

User draws to the screen by rendering to framebuffer
the texture is then rendered onto a quad

*/

class Simulation
{
private:
	static constexpr float quad[] = 
	{
		1.0f,  1.0f, 	1.0f, 1.0f,// top right
		1.0f, -1.0f,	1.0f, 0.0f,// bottom right
	   -1.0f, -1.0f,	0.0f, 0.0f,// bottom left
	   -1.0f,  1.0f,	0.0f, 1.0f,// top left 
	};

	static constexpr unsigned int quadIndices[] =
	{
		0, 1, 3,
		1, 2, 3,
	};

	static constexpr const char* INPUT_UNIFORM = "textureIn";

	class Shader
	{
	public:
		Shader() = default;
		Shader(const char* vertexPath, const char* fragmentPath);

		unsigned int GetId() const;

		void Use() const;

		void SetBool(const std::string& name, bool value) const;
		void SetInt(const std::string& name, int value) const;
		void SetFloat(const std::string& name, float value) const;
		void SetVec2(const std::string& name, float f0, float f1) const;
		void SetVec4(const std::string& name, float f0, float f1, float f2, float f3) const;
		void SetVec3(const std::string& name, float f0, float f1, float f2) const;
		void SetMat4(const std::string& name, glm::mat4& mat);

	private:
		unsigned int id;
	};

	struct Uniforms;
	class GUIHandler
	{
	public:
		GUIHandler() = default;
		GUIHandler(GLFWwindow* window, Uniforms& uniforms, glm::vec3& color);
		GUIHandler(Uniforms& uniforms, glm::vec3& color);

		void Init();
		void RenderStart();
		void CreateGui();
		void RenderEnd();
		void Shutdown();

		void SetWindow(GLFWwindow* window) { this->window = window; }
		ImGuiIO* GetIO() const { return io; }

	private:
		GLFWwindow* window;
		Uniforms& uniforms;
		glm::vec3& color;
		ImGuiIO* io = nullptr;
	} gui;

public:
	Simulation(const std::string& vertexShader, const std::string& simVertShader, const std::string& fragmentShader, const std::string& passthroughFrag, const std::string& brushFrag, unsigned int resolutionX, unsigned int resolutionY, unsigned int windowWidth, unsigned int windowHeight);

	void Init();
	void MainLoop();

private:
	void InitGLFW();
	void InitQuad();
	void InitRendering();
	void processInput();

	void DrawPixels(double x, double y);

private:
	struct Uniforms
	{
		float ri = 3.0f;
		float ra = 13.0f;
		float dt = 0.4f;
		float alpha_m = 0.147f;

		float alpha_n = 0.028f;
		float b1 = 0.261f;
		float b2 = 0.312f;
		float d1 = 0.327f;

		float d2 = 0.544f;
	} uniforms;

	glm::vec3 color{ 92.0f/255.0f ,176.0f / 255.0f ,255.0f / 255.0f };

private:
	std::string vertp;
	std::string fragp;
	std::string passp;
	std::string brushp;
	std::string simvp;

	Shader shader{};
	Shader passthrough{};
	Shader brush{};

	unsigned int resX;
	unsigned int resY;

	unsigned int width;
	unsigned int height;

	unsigned int vao = (unsigned int)-1;
	unsigned int vbo = (unsigned int)-1;
	unsigned int ebo = (unsigned int)-1;

	unsigned int texture0 = (unsigned int)-1;
	unsigned int fbo = (unsigned int)-1;
	unsigned int fbo2 = (unsigned int)-1;
	unsigned int texture1 = (unsigned int)-1;

	unsigned int ubo = (unsigned int)-1;

	GLFWwindow* window = nullptr;
};