#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

		void Use();

		void SetBool(const std::string& name, bool value) const;
		void SetInt(const std::string& name, int value) const;
		void SetFloat(const std::string& name, float value) const;
		void SetVec2(const std::string& name, float f0, float f1);
		void SetVec4(const std::string& name, float f0, float f1, float f2, float f3);
		void SetVec3(const std::string& name, float f0, float f1, float f2);
		void SetMat4(const std::string& name, glm::mat4& mat);

	private:
		unsigned int id;
	};
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

	GLFWwindow* window = nullptr;
};