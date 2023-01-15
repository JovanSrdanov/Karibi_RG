#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <thread>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include "shader.hpp"
#include "model.hpp"
#include "renderable.hpp"

const int WindowWidth = 1280;
const int WindowHeight = 720;
const std::string WindowTitle = "Karibi";
const float TargetFPS = 60.0f;
const float TargetFrameTime = 1.0f / TargetFPS;

void FramebufferSizeCallback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	bool IsDown = action == GLFW_PRESS || action == GLFW_REPEAT;
	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	}
}

static void ErrorCallback(int error, const char *description)
{
	std::cerr << "GLFW Error: " << description << std::endl;
}

int main()
{
	GLFWwindow *Window = 0;
	if (!glfwInit())
	{
		std::cerr << "Failed to init glfw" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwSetErrorCallback(ErrorCallback);

	Window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle.c_str(), 0, 0);
	if (!Window)
	{
		std::cerr << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(Window);
	glfwSetKeyCallback(Window, KeyCallback);
	glfwSetFramebufferSizeCallback(Window, FramebufferSizeCallback);

	GLenum GlewError = glewInit();
	if (GlewError != GLEW_OK)
	{
		std::cerr << "Failed to init glew: " << glewGetErrorString(GlewError) << std::endl;
		glfwTerminate();
		return -1;
	}

	Shader Basic("shaders/basic.vert", "shaders/basic.frag");

	Model IronMan("IronMan/IronMan.obj");
	if (!IronMan.Load())
	{
		std::cout << "Failed to load model!\n";
		glfwTerminate();
		return -1;
	}

	float cubeVertices[] =
		{
			-0.2, -0.2, 0.2, 0, 0, 0,
			0.2, -0.2, 0.2, 0, 0, 0,
			0.2, 0.2, 0.2, 0, 0, 0,
			-0.2, 0.2, 0.2, 0, 0, 0,

			-0.2, -0.2, -0.2, 0, 0, 0,
			0.2, -0.2, -0.2, 0, 0, 0,
			0.2, 0.2, -0.2, 0, 0, 0,
			-0.2, 0.2, -0.2, 0, 0, 0};

	unsigned int cubeIndices[] =
		{
			0, 1, 2,
			2, 3, 0,

			1, 5, 6,
			6, 2, 1,

			7, 6, 5,
			5, 4, 7,

			4, 0, 3,
			3, 7, 4,

			4, 5, 1,
			1, 0, 4,

			3, 2, 6,
			6, 7, 3};
	float triangleVertices[] =
		{
			-0.5f,
			-0.5f,
			0.0f,
			0.0,
			0.0,
			0.0,
			0.5f,
			-0.5f,
			0.0f,
			0.0,
			0.0,
			0.0,
			0.0f,
			0.5f,
			0.0f,
			0.0,
			0.0,
			0.0,
		};

	unsigned int triangleIndices[] =
		{
			0,
			1,
			2,
			1,
			2,
			3,
		};

	Renderable cube(cubeVertices, sizeof(cubeVertices), cubeIndices, sizeof(cubeIndices));
	Renderable triangle(triangleVertices, sizeof(triangleVertices), triangleIndices, sizeof(triangleIndices));

	glm::mat4 m(1.0f);
	glm::mat4 v = glm::lookAt(glm::vec3(0.0, 1.0, -2.2), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 p = glm::perspective(glm::radians(90.0f), (float)WindowWidth / WindowHeight, 0.1f, 100.0f);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.53, 0.81, 0.98, 1.0);

	float FrameStartTime = glfwGetTime();
	float FrameEndTime = glfwGetTime();
	float dt = FrameEndTime - FrameStartTime;
	bool clouds_visibility = true;
	bool culling = true;

	while (!glfwWindowShouldClose(Window))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Exit buttons
		if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(Window, GL_TRUE);
		}

		// Cloud visibility buttons
		if (glfwGetKey(Window, GLFW_KEY_P) == GLFW_PRESS)
		{
			clouds_visibility = false;
		}
		if (glfwGetKey(Window, GLFW_KEY_O) == GLFW_PRESS)
		{
			clouds_visibility = true;
		}

		// Culling buttons
		if (glfwGetKey(Window, GLFW_KEY_L) == GLFW_PRESS)
		{
			culling = false;
		}
		if (glfwGetKey(Window, GLFW_KEY_K) == GLFW_PRESS)
		{
			culling = true;
		}

		FrameStartTime = glfwGetTime();
		glUseProgram(Basic.GetId());
		Basic.SetProjection(p);
		Basic.SetView(v);

		if (culling)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}

		// Sun part 1
		Basic.SetColor(1, 0.5 + abs(sin(glfwGetTime() * 5)), 0);
		m = glm::translate(glm::mat4(1.0f), glm::vec3(+1.9, 1.2, -0.4));
		m = glm::rotate(m, glm::radians((float)glfwGetTime() * 30), glm::vec3(1.0, 0, 1));
		Basic.SetModel(m);
		cube.Render();

		// Sun part 2
		Basic.SetColor(1, 0.55 + abs(sin(glfwGetTime() * 7)), 0.3);
		m = glm::translate(glm::mat4(1.0f), glm::vec3(+1.9, 1.2, -0.4));
		m = glm::rotate(m, glm::radians((float)glfwGetTime() * 20), glm::vec3(-1.0, 1.0, 0));
		Basic.SetModel(m);
		cube.Render();

		// Clouds
		if (clouds_visibility)
		{
			// Left cloud
			Basic.SetColor(1, 1, 1);
			m = glm::translate(glm::mat4(1.0f), glm::vec3(1.0, 1.3, 0.1));
			m = glm::rotate(m, glm::radians(135.0f), glm::vec3(10, 0, 1.0));
			m = glm::scale(m, glm::vec3(1.0, 1.0, 2.0));
			Basic.SetModel(m);
			cube.Render();

			// Right cloud
			Basic.SetColor(0.9, 0.9, 0.9);
			m = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0, 1.5, 0.1));
			m = glm::rotate(m, glm::radians(15.0f), glm::vec3(1.0, 1.0, 1.0));
			m = glm::scale(m, glm::vec3(1.50, 0.5, 2.0));
			Basic.SetModel(m);
			cube.Render();
		}

		// Sea
		Basic.SetColor(0, 0, 1);
		m = glm::translate(glm::mat4(1.0f), glm::vec3(0, (abs(sin(glfwGetTime() * 2)) / 5) - 1.5, 0.0));
		m = glm::scale(m, glm::vec3(55.0, 5.0, 15.0));
		Basic.SetModel(m);
		cube.Render();

		// Right island
		Basic.SetColor(0.43, 0.17, 0.00);
		m = glm::rotate(m, glm::radians(115.0f), glm::vec3(0, 10, 0));
		m = glm::translate(glm::mat4(1.0f), glm::vec3(-3, -0.2, 0));
		m = glm::scale(m, glm::vec3(1.5));
		Basic.SetModel(m);
		cube.Render();

		// Left island
		Basic.SetColor(0.43, 0.17, 0.00);
		m = glm::translate(glm::mat4(1.0f), glm::vec3(3, -0.4, 0));
		m = glm::rotate(m, glm::radians((4.0f * 10)), glm::vec3(0, 1, 0));
		m = glm::scale(m, glm::vec3(2.5, 2.5, 2.5));
		Basic.SetModel(m);
		cube.Render();

		// Center island
		Basic.SetColor(0.43, 0.17, 0.00);
		m = glm::rotate(m, glm::radians(30.0f), glm::vec3(0, 1, 0));
		m = glm::translate(glm::mat4(1.0f), glm::vec3(0, -0.9, -0.5));
		m = glm::scale(m, glm::vec3(5));
		Basic.SetModel(m);
		cube.Render();

		// Palm tree
		Basic.SetColor(0.75, 0.43, 0.12);
		m = glm::translate(glm::mat4(1.0f), glm::vec3(0, -0.2, -0.5));
		m = glm::rotate(m, glm::radians(30.0f), glm::vec3(0, 1, 0));
		m = glm::scale(m, glm::vec3(0.5, 7, 0.5));
		Basic.SetModel(m);
		cube.Render();

		// Iron Man model
		Basic.SetColor(1, 0, 0);
		m = glm::translate(glm::mat4(1.0f), glm::vec3(0.2, 0.20, -1.0));
		m = glm::scale(m, glm::vec3(0.002));
		Basic.SetModel(m);
		IronMan.Render();

		// Top of the palm tree
		Basic.SetColor(0.00, 0.31, 0.04);
		m = glm::translate(glm::mat4(1.0f), glm::vec3(0, 1.2, -0.5));
		m = glm::rotate(m, glm::radians(30.0f), glm::vec3(0, 1, 0));
		m = glm::scale(m, glm::vec3(0.6, 0.4, 0.6));
		Basic.SetModel(m);
		cube.Render();

		// Left leaf of the palm tree
		Basic.SetColor(0.00, 0.31, 0.04);
		m = glm::translate(glm::mat4(1.0f), glm::vec3(0.2, 1.05, -0.5));
		m = glm::scale(m, glm::vec3(0.5, 0.7, 1));
		m = glm::rotate(m, glm::radians(120.0f), glm::vec3(0, 1, 0));
		m = glm::rotate(m, glm::radians(-40.0f), glm::vec3(1, 0, 0));
		Basic.SetModel(m);
		triangle.Render();

		// Right leaf of the palm tree
		Basic.SetColor(0.00, 0.31, 0.04);
		m = glm::translate(glm::mat4(1.0f), glm::vec3(-0.2, 1.06, -0.5));
		m = glm::scale(m, glm::vec3(0.5, 0.7, 1));
		m = glm::rotate(m, glm::radians(230.0f), glm::vec3(0, 1, 0));
		m = glm::rotate(m, glm::radians(-40.0f), glm::vec3(1, 0, 0));
		Basic.SetModel(m);
		triangle.Render();

		// Center leaf of the palm tree
		Basic.SetColor(0.00, 0.31, 0.04);
		m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 1.06, -0.7));
		m = glm::scale(m, glm::vec3(0.2, 0.7, 1));
		m = glm::rotate(m, glm::radians(190.0f), glm::vec3(0, 1, 0));
		m = glm::rotate(m, glm::radians(-40.0f), glm::vec3(1, 0, 0));
		Basic.SetModel(m);
		triangle.Render();

		if (culling)
		{
			glDisable(GL_CULL_FACE);
		}

		glUseProgram(0);
		glfwSwapBuffers(Window);

		FrameEndTime = glfwGetTime();
		dt = FrameEndTime - FrameStartTime;
		if (dt < TargetFPS)
		{
			int DeltaMS = (int)((TargetFrameTime - dt) * 1e3f);
			std::this_thread::sleep_for(std::chrono::milliseconds(DeltaMS));
		}
	}
	glfwTerminate();
	return 0;
}