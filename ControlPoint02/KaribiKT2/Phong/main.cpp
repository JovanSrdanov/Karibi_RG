#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <thread>
#include "shader.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "texture.hpp"

struct Input
{
	bool MoveLeft;
	bool MoveRight;
	bool MoveUp;
	bool MoveDown;
	bool LookLeft;
	bool LookRight;
	bool LookUp;
	bool LookDown;
	bool GoUp;
	bool GoDown;
};

struct EngineState
{
	Input* mInput;
	Camera* mCamera;
	unsigned mShadingMode;
	bool mDrawDebugLines;
	double mDT;
};

static void ErrorCallback(int error, const char* description)
{
	std::cerr << "GLFW Error: " << description << std::endl;
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	EngineState* State = (EngineState*)glfwGetWindowUserPointer(window);
	Input* UserInput = State->mInput;
	bool IsDown = action == GLFW_PRESS || action == GLFW_REPEAT;
	switch (key)
	{
	case GLFW_KEY_A: UserInput->MoveLeft = IsDown; break;
	case GLFW_KEY_D: UserInput->MoveRight = IsDown; break;
	case GLFW_KEY_W: UserInput->MoveUp = IsDown; break;
	case GLFW_KEY_S: UserInput->MoveDown = IsDown; break;

	case GLFW_KEY_RIGHT: UserInput->LookLeft = IsDown; break;
	case GLFW_KEY_LEFT: UserInput->LookRight = IsDown; break;
	case GLFW_KEY_UP: UserInput->LookUp = IsDown; break;
	case GLFW_KEY_DOWN: UserInput->LookDown = IsDown; break;

	case GLFW_KEY_SPACE: UserInput->GoUp = IsDown; break;
	case GLFW_KEY_C: UserInput->GoDown = IsDown; break;

	case GLFW_KEY_L:
	{
		if (IsDown)
		{
			State->mDrawDebugLines ^= true; break;
		}
	} break;

	case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
	}
}

static void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

static void HandleInput(EngineState* state)
{
	Input* UserInput = state->mInput;
	Camera* FPSCamera = state->mCamera;
	if (UserInput->MoveLeft) FPSCamera->Move(-1.0f, 0.0f, state->mDT);
	if (UserInput->MoveRight) FPSCamera->Move(1.0f, 0.0f, state->mDT);
	if (UserInput->MoveDown) FPSCamera->Move(0.0f, -1.0f, state->mDT);
	if (UserInput->MoveUp) FPSCamera->Move(0.0f, 1.0f, state->mDT);

	if (UserInput->LookLeft) FPSCamera->Rotate(1.0f, 0.0f, state->mDT);
	if (UserInput->LookRight) FPSCamera->Rotate(-1.0f, 0.0f, state->mDT);
	if (UserInput->LookDown) FPSCamera->Rotate(0.0f, -1.0f, state->mDT);
	if (UserInput->LookUp) FPSCamera->Rotate(0.0f, 1.0f, state->mDT);

	if (UserInput->GoUp) FPSCamera->UpDown(1);
	if (UserInput->GoDown) FPSCamera->UpDown(-1);
}

static void DrawSea(unsigned vao, const Shader& shader, unsigned diffuse, unsigned specular, double time)
{
	glUseProgram(shader.GetId());
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuse);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specular);
	constexpr int sea_size = 10;
	for (int i = -sea_size; i < sea_size; ++i)
	{
		for (int j = -sea_size; j < sea_size; ++j)
		{
			constexpr float size = 4.0f;
			glm::mat4 model_matrix(1.0f);

			// Waves
			model_matrix = glm::translate(model_matrix, glm::vec3(i * size, (abs(sin(time))) - size * 1.6, j * size));
			model_matrix = glm::rotate(model_matrix, glm::radians(static_cast<float>(time * (45 + i))), glm::vec3(0.11, 0, 2));
			model_matrix = glm::scale(model_matrix, glm::vec3(size, size, size));
			shader.SetModel(model_matrix);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			// Steady sea
			model_matrix = glm::mat4(1);
			model_matrix = glm::translate(model_matrix, glm::vec3(i * size, (abs(sin(time))) - size * 1.5, j * size));
			model_matrix = glm::scale(model_matrix, glm::vec3(size, size, size));
			shader.SetModel(model_matrix);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
}

int main()
{
	GLFWwindow* Window = 0;
	if (!glfwInit())
	{
		std::cerr << "Failed to init glfw" << std::endl;
		return -1;
	}

	const std::string WindowTitle = "Karibi Kontrolna Tacka 02";
	int WindowWidth = glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
	int WindowHeight = glfwGetVideoMode(glfwGetPrimaryMonitor())->height;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);

	Window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle.c_str(), 0, 0);
	if (!Window)
	{
		std::cerr << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(Window);

	GLenum GlewError = glewInit();
	if (GlewError != GLEW_OK)
	{
		std::cerr << "Failed to init glew: " << glewGetErrorString(GlewError) << std::endl;
		glfwTerminate();
		return -1;
	}

	EngineState State = { 0 };
	Camera FPSCamera;
	Input UserInput = { 0 };
	State.mCamera = &FPSCamera;
	State.mInput = &UserInput;
	glfwSetWindowUserPointer(Window, &State);

	glfwSetErrorCallback(ErrorCallback);
	glfwSetFramebufferSizeCallback(Window, FramebufferSizeCallback);
	glfwSetKeyCallback(Window, KeyCallback);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	Model woman("res/Woman/091_W_Aya_100K.obj");
	if (!woman.Load())
	{
		std::cerr << "Failed to load model\n";
		glfwTerminate();
		return -1;
	}
	Model shark("res/Shark/SHARK.obj");
	if (!shark.Load())
	{
		std::cerr << "Failed to load model\n";
		glfwTerminate();
		return -1;
	}

	std::vector<float> CubeVertices =
	{
		// X     Y     Z     NX    NY    NZ    U     V    FRONT SIDE
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // L D
		 0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // R D
		-0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // L U
		 0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // R D
		 0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // R U
		-0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // L U
														// LEFT SIDE
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // L D
		-0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
		-0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
		-0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
		-0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // R U
		-0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
														// RIGHT SIDE
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // L D
		 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
		 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
		 0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // R U
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
														// BOTTOM SIDE
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // L D
		 0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // R D
		-0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // L U
		 0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // R D
		 0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // R U
		-0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // L U
														// TOP SIDE
		-0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // L D
		 0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // R D
		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // L U
		 0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // R D
		 0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // R U
		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // L U
														// BACK SIDE
		 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // L D
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // R D
		 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // L U
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // R D
		-0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // R U
		 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // L U
	};

	unsigned CubeVAO;
	glGenVertexArrays(1, &CubeVAO);
	glBindVertexArray(CubeVAO);
	unsigned CubeVBO;
	glGenBuffers(1, &CubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, CubeVBO);
	glBufferData(GL_ARRAY_BUFFER, CubeVertices.size() * sizeof(float), CubeVertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), static_cast<void*>(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	Shader PhongShaderMaterialTexture("shaders/basic.vert", "shaders/phong_material_texture.frag");
	glUseProgram(PhongShaderMaterialTexture.GetId());

	// Light from far away
	PhongShaderMaterialTexture.SetUniform3f("uDirLight.Ka", glm::vec3(1.00, 0.97, 0.00));
	PhongShaderMaterialTexture.SetUniform3f("uDirLight.Kd", glm::vec3(1.00, 0.97, 0.00));
	PhongShaderMaterialTexture.SetUniform3f("uDirLight.Ks", glm::vec3(1.0f));

	// Default for point light (Sun)
	PhongShaderMaterialTexture.SetUniform3f("uSunLight.Position", glm::vec3(-999));
	PhongShaderMaterialTexture.SetUniform3f("uSunLight.Ka", glm::vec3(1.00, 0.97, 0.00));
	PhongShaderMaterialTexture.SetUniform3f("uSunLight.Kd", glm::vec3(1.00, 0.97, 0.00));
	PhongShaderMaterialTexture.SetUniform3f("uSunLight.Ks", glm::vec3(0));
	PhongShaderMaterialTexture.SetUniform1f("uSunLight.Kc", 1.0f);
	PhongShaderMaterialTexture.SetUniform1f("uSunLight.Kl", 0.01f);
	PhongShaderMaterialTexture.SetUniform1f("uSunLight.Kq", 0.005f);

	// Default for point light (Torch 1)
	PhongShaderMaterialTexture.SetUniform3f("uTorchLight1.Ka", glm::vec3(1.00, 0.44, 0.00));
	PhongShaderMaterialTexture.SetUniform3f("uTorchLight1.Kd", glm::vec3(1.00, 0.44, 0.00));
	PhongShaderMaterialTexture.SetUniform3f("uTorchLight1.Ks", glm::vec3(1.00, 0.44, 0.00));
	PhongShaderMaterialTexture.SetUniform1f("uTorchLight1.Kc", 1.0f);
	PhongShaderMaterialTexture.SetUniform1f("uTorchLight1.Kl", 0.01f);
	PhongShaderMaterialTexture.SetUniform1f("uTorchLight1.Kq", 0.005f);

	// Default for point light (Torch 2)
	PhongShaderMaterialTexture.SetUniform3f("uTorchLight2.Ka", glm::vec3(1.00, 0.44, 0.00));
	PhongShaderMaterialTexture.SetUniform3f("uTorchLight2.Kd", glm::vec3(1.00, 0.44, 0.00));
	PhongShaderMaterialTexture.SetUniform3f("uTorchLight2.Ks", glm::vec3(1.00, 0.44, 0.00));
	PhongShaderMaterialTexture.SetUniform1f("uTorchLight2.Kc", 1.0f);
	PhongShaderMaterialTexture.SetUniform1f("uTorchLight2.Kl", 0.01f);
	PhongShaderMaterialTexture.SetUniform1f("uTorchLight2.Kq", 0.005f);

	// Default for point light (Torch 3)
	PhongShaderMaterialTexture.SetUniform3f("uTorchLight3.Ka", glm::vec3(1.00, 0.44, 0.00));
	PhongShaderMaterialTexture.SetUniform3f("uTorchLight3.Kd", glm::vec3(1.00, 0.44, 0.00));
	PhongShaderMaterialTexture.SetUniform3f("uTorchLight3.Ks", glm::vec3(1.00, 0.44, 0.00));
	PhongShaderMaterialTexture.SetUniform1f("uTorchLight3.Kc", 1.0f);
	PhongShaderMaterialTexture.SetUniform1f("uTorchLight3.Kl", 0.01f);
	PhongShaderMaterialTexture.SetUniform1f("uTorchLight3.Kq", 0.005f);

	// Default for point light (Lighthouse top)
	PhongShaderMaterialTexture.SetUniform3f("uLightHousePointLight.Ka", glm::vec3(1));
	PhongShaderMaterialTexture.SetUniform3f("uLightHousePointLight.Kd", glm::vec3(1));
	PhongShaderMaterialTexture.SetUniform3f("uLightHousePointLight.Ks", glm::vec3(1));
	PhongShaderMaterialTexture.SetUniform1f("uLightHousePointLight.Kc", 1.0f);
	PhongShaderMaterialTexture.SetUniform1f("uLightHousePointLight.Kl", 0.5f);
	PhongShaderMaterialTexture.SetUniform1f("uLightHousePointLight.Kq", 0.7f);

	// First light from the Lighthouse 
	PhongShaderMaterialTexture.SetUniform3f("uLighthouseLight1.Position", glm::vec3(999));
	PhongShaderMaterialTexture.SetUniform3f("uLighthouseLight1.Direction", glm::vec3(999));
	PhongShaderMaterialTexture.SetUniform3f("uLighthouseLight1.Ka", glm::vec3(0));
	PhongShaderMaterialTexture.SetUniform3f("uLighthouseLight1.Kd", glm::vec3(1));
	PhongShaderMaterialTexture.SetUniform3f("uLighthouseLight1.Ks", glm::vec3(1.0f));
	PhongShaderMaterialTexture.SetUniform1f("uLighthouseLight1.Kc", 1.0f);
	PhongShaderMaterialTexture.SetUniform1f("uLighthouseLight1.Kl", 0.0002f);
	PhongShaderMaterialTexture.SetUniform1f("uLighthouseLight1.Kq", 0.0002f);
	PhongShaderMaterialTexture.SetUniform1f("uLighthouseLight1.InnerCutOff", glm::cos(glm::radians(10.0f)));
	PhongShaderMaterialTexture.SetUniform1f("uLighthouseLight1.OuterCutOff", glm::cos(glm::radians(35.0f)));

	// Second light from the Lighthouse 
	PhongShaderMaterialTexture.SetUniform3f("uLighthouseLight2.Position", glm::vec3(999));
	PhongShaderMaterialTexture.SetUniform3f("uLighthouseLight2.Direction", glm::vec3(999));
	PhongShaderMaterialTexture.SetUniform3f("uLighthouseLight2.Ka", glm::vec3(0));
	PhongShaderMaterialTexture.SetUniform3f("uLighthouseLight2.Kd", glm::vec3(1));
	PhongShaderMaterialTexture.SetUniform3f("uLighthouseLight2.Ks", glm::vec3(1));
	PhongShaderMaterialTexture.SetUniform1f("uLighthouseLight2.Kc", 1.0f);
	PhongShaderMaterialTexture.SetUniform1f("uLighthouseLight2.Kl", 0.0002f);
	PhongShaderMaterialTexture.SetUniform1f("uLighthouseLight2.Kq", 0.0002f);
	PhongShaderMaterialTexture.SetUniform1f("uLighthouseLight2.InnerCutOff", glm::cos(glm::radians(10.0f)));
	PhongShaderMaterialTexture.SetUniform1f("uLighthouseLight2.OuterCutOff", glm::cos(glm::radians(35.0f)));

	// Light from the FlashLight 
	PhongShaderMaterialTexture.SetUniform3f("uFlashLight.Position", glm::vec3(999));
	PhongShaderMaterialTexture.SetUniform3f("uFlashLight.Direction", glm::vec3(999));
	PhongShaderMaterialTexture.SetUniform3f("uFlashLight.Ka", glm::vec3(0));
	PhongShaderMaterialTexture.SetUniform3f("uFlashLight.Kd", glm::vec3(1));
	PhongShaderMaterialTexture.SetUniform3f("uFlashLight.Ks", glm::vec3(1));
	PhongShaderMaterialTexture.SetUniform1f("uFlashLight.Kc", 0.7f);
	PhongShaderMaterialTexture.SetUniform1f("uFlashLight.Kl", 0.0002f);
	PhongShaderMaterialTexture.SetUniform1f("uFlashLight.Kq", 0.0002f);
	PhongShaderMaterialTexture.SetUniform1f("uFlashLight.InnerCutOff", glm::cos(glm::radians(1.0f)));
	PhongShaderMaterialTexture.SetUniform1f("uFlashLight.OuterCutOff", glm::cos(glm::radians(30.0f)));

	// Materials
	PhongShaderMaterialTexture.SetUniform1i("uMaterial.Kd", 0);
	PhongShaderMaterialTexture.SetUniform1i("uMaterial.Ks", 1);
	PhongShaderMaterialTexture.SetUniform1f("uMaterial.Shininess", 64);

	// Diffuse texture
	unsigned SunDiffuseTexture = Texture::LoadImageToTexture("res/sun.jpg");
	unsigned SandDiffuseTexture = Texture::LoadImageToTexture("res/sand.jpg");
	unsigned RockDiffuseTexture = Texture::LoadImageToTexture("res/rock.jpg");
	unsigned LighthouseDiffuseTexture = Texture::LoadImageToTexture("res/lighthouse.jpg");
	unsigned LighthouseLampDiffuseTexture = Texture::LoadImageToTexture("res/lighthouseLamp_d.jpg");
	unsigned CloudDiffuseTexture = Texture::LoadImageToTexture("res/cloud.jpg");
	unsigned PalmTreeDiffuseTexture = Texture::LoadImageToTexture("res/palmTree.jpg");
	unsigned PalmLeafDiffuseTexture = Texture::LoadImageToTexture("res/palmLeaf.jpg");
	unsigned CampfireDiffuseTexture = Texture::LoadImageToTexture("res/campfire.jpg");
	unsigned SeaDiffuseTexture = Texture::LoadImageToTexture("res/sea_d.jpg");

	// Specular texture
	unsigned SeaSpecularTexture = Texture::LoadImageToTexture("res/sea_s.jpg");
	unsigned LighthouseLampSpecularTexture = Texture::LoadImageToTexture("res/lighthouseLamp_s.jpg");

	// Start values of variables
	Shader* CurrentShader = &PhongShaderMaterialTexture;
	bool clouds_and_lighthouse_light_visibility = true;
	bool is_day = true;
	bool flash_light = false;
	double pi = atan(1) * 4;
	double start_time;
	glm::mat4 model_matrix(1.0f);
	glClearColor(0.53f, 0.81f, 0.98f, 1.0f);


	while (!glfwWindowShouldClose(Window)) {
		start_time = glfwGetTime();
		glfwPollEvents();
		HandleInput(&State);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(CurrentShader->GetId());
		CurrentShader->SetProjection(glm::perspective(90.0f, static_cast<float>(WindowWidth) / static_cast<float>(WindowHeight), 0.1f, 100.0f));
		CurrentShader->SetView(glm::lookAt(FPSCamera.GetPosition(), FPSCamera.GetTarget(), FPSCamera.GetUp()));
		CurrentShader->SetUniform3f("uViewPos", FPSCamera.GetPosition());

		if (glfwGetKey(Window, GLFW_KEY_P) == GLFW_PRESS)
		{
			clouds_and_lighthouse_light_visibility = false;
		}
		if (glfwGetKey(Window, GLFW_KEY_O) == GLFW_PRESS)
		{
			clouds_and_lighthouse_light_visibility = true;
		}

		if (glfwGetKey(Window, GLFW_KEY_K) == GLFW_PRESS)
		{
			is_day = false;
		}
		if (glfwGetKey(Window, GLFW_KEY_L) == GLFW_PRESS)
		{
			is_day = true;
		}

		if (glfwGetKey(Window, GLFW_KEY_F) == GLFW_PRESS)
		{
			flash_light = true;
		}
		if (glfwGetKey(Window, GLFW_KEY_G) == GLFW_PRESS)
		{
			flash_light = false;
		}

		if (flash_light)
		{
			glm::vec3 pos = FPSCamera.GetTarget() - FPSCamera.GetPosition();
			CurrentShader->SetUniform3f("uFlashLight.Position", glm::vec3(FPSCamera.GetPosition()));
			CurrentShader->SetUniform3f("uFlashLight.Direction", glm::vec3(pos.x, pos.y, pos.z));
		}
		if (!flash_light)
		{
			CurrentShader->SetUniform3f("uFlashLight.Position", glm::vec3(-999));
			CurrentShader->SetUniform3f("uFlashLight.Direction", glm::vec3(-998));
		}

		if (is_day)
		{
			glClearColor(0.53f, 0.81f, 0.98f, 1.0f);
			CurrentShader->SetUniform3f("uDirLight.Ka", glm::vec3(0.68, 0.70, 0.51));
			CurrentShader->SetUniform3f("uDirLight.Kd", glm::vec3(0.68, 0.70, 0.51));
			CurrentShader->SetUniform3f("uDirLight.Ks", glm::vec3(1.0f));

			// Sun
			CurrentShader->SetUniform3f("uDirLight.Direction", glm::vec3(0, -0.1, 0));

			glm::vec3 point_light_position_sun(0, 25, 0);
			PhongShaderMaterialTexture.SetUniform1f("uSunLight.Kc", 0.1 / abs(sin(start_time)));

			PhongShaderMaterialTexture.SetUniform1f("uSunLight.Kq", 0.1 / abs(sin(start_time)));
			CurrentShader->SetUniform3f("uSunLight.Position", point_light_position_sun);
			model_matrix = glm::mat4(1.0f);
			model_matrix = glm::translate(model_matrix, point_light_position_sun);
			model_matrix = glm::scale(model_matrix, glm::vec3(7));
			CurrentShader->SetModel(model_matrix);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, SunDiffuseTexture);
			glBindVertexArray(CubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);
		}

		if (!is_day)
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			double far_far_away_light = 0;
			CurrentShader->SetUniform3f("uDirLight.Ka", glm::vec3(far_far_away_light));
			CurrentShader->SetUniform3f("uDirLight.Kd", glm::vec3(far_far_away_light));
			CurrentShader->SetUniform3f("uDirLight.Ks", glm::vec3(far_far_away_light));
			CurrentShader->SetUniform3f("uSunLight.Position", glm::vec3(999));

			// Sharks
			int number_of_sharks = 4;
			for (int i = 0; i < number_of_sharks; i++)
			{
				float angle_of_shark = (2 * pi / number_of_sharks) * i;
				float distance_from_island = 10;
				model_matrix = glm::mat4(1.0f);
				model_matrix = glm::translate(model_matrix, glm::vec3(distance_from_island * sin(angle_of_shark + start_time), -4.25f, distance_from_island * cos(angle_of_shark + start_time)));
				model_matrix = glm::scale(model_matrix, glm::vec3(1.5));
				model_matrix = glm::rotate(model_matrix, glm::radians(static_cast<float>(start_time) * 100), glm::vec3(0, 1, 0));
				model_matrix = glm::rotate(model_matrix, glm::radians(-45.0f), glm::vec3(0, 0, 1));
				CurrentShader->SetModel(model_matrix);
				shark.Render();
			}

		}

		// Sea
		DrawSea(CubeVAO, *CurrentShader, SeaDiffuseTexture, SeaSpecularTexture, start_time);

		// Small island (Far)
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, glm::vec3(25.0f, -2.7f, 25.0f));
		model_matrix = glm::scale(model_matrix, glm::vec3(4));
		CurrentShader->SetModel(model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SandDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Torch on small island (Far)
		glm::vec3 PointLightPositionTorch1(25.0f, -0.7, 25.0f);
		CurrentShader->SetUniform3f("uTorchLight1.Position", PointLightPositionTorch1);
		CurrentShader->SetUniform1f("uTorchLight1.Kc", 0.1 / abs(sin(start_time * 2)));
		CurrentShader->SetUniform1f("uTorchLight1.Kl", 0.1 / abs(sin(start_time * 3)));
		CurrentShader->SetUniform1f("uTorchLight1.Kq", 1.0 / abs(sin(start_time * 5)));
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, PointLightPositionTorch1);
		model_matrix = glm::scale(model_matrix, glm::vec3(1));
		CurrentShader->SetModel(model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, CampfireDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Small island (Near)
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, glm::vec3(-20.0f, -2.7f, -15.0f));
		model_matrix = glm::scale(model_matrix, glm::vec3(4));
		CurrentShader->SetModel(model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SandDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Torch on small island (Near)
		glm::vec3 PointLightPositionSunTorch2(-20.0f, -0.7f, -15.0f);
		CurrentShader->SetUniform3f("uTorchLight2.Position", PointLightPositionSunTorch2);
		CurrentShader->SetUniform1f("uTorchLight2.Kc", 0.1 / abs(sin(start_time * 2)));
		CurrentShader->SetUniform1f("uTorchLight2.Kl", 0.091 / abs(sin(start_time * 3)));
		CurrentShader->SetUniform1f("uTorchLight2.Kq", 0.1 / abs(sin(start_time * 5)));
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, PointLightPositionSunTorch2);
		model_matrix = glm::scale(model_matrix, glm::vec3(1));
		CurrentShader->SetModel(model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, CampfireDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Big island
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, -3.0f, 0.0f));
		model_matrix = glm::scale(model_matrix, glm::vec3(10, 3, 10));
		CurrentShader->SetModel(model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SandDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Torch on big island
		glm::vec3 PointLightPositionSunTorch3(3.5f, -1.4f, 3.5f);
		CurrentShader->SetUniform3f("uTorchLight3.Position", PointLightPositionSunTorch3);
		CurrentShader->SetUniform1f("uTorchLight3.Kc", 1 / abs(sin(start_time * 2)));
		CurrentShader->SetUniform1f("uTorchLight3.Kl", 0.1 / abs(sin(start_time * 3)));
		CurrentShader->SetUniform1f("uTorchLight3.Kq", 0.1 / abs(sin(start_time * 5)));
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, PointLightPositionSunTorch3);
		model_matrix = glm::scale(model_matrix, glm::vec3(1));
		CurrentShader->SetModel(model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, CampfireDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Model on island (Woman)
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, glm::vec3(-4.5f, -2.25f, -4.5f));
		model_matrix = glm::scale(model_matrix, glm::vec3(0.002));
		model_matrix = glm::rotate(model_matrix, glm::radians(155.0f), glm::vec3(0, 1, 0));
		CurrentShader->SetModel(model_matrix);
		woman.Render();

		// Palm tree
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, 1.5f, 0.0f));
		model_matrix = glm::scale(model_matrix, glm::vec3(1, 10, 1));
		CurrentShader->SetModel(model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, PalmTreeDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Palm tree top leaf
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, 6.0f, 0.0f));
		model_matrix = glm::scale(model_matrix, glm::vec3(2));
		model_matrix = glm::rotate(model_matrix, glm::radians(45.0f), glm::vec3(0, 1, 0));
		CurrentShader->SetModel(model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, PalmLeafDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Palm tree leaf 1
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, glm::vec3(1.5f, 4.75f, -1.5f));
		model_matrix = glm::rotate(model_matrix, glm::radians(45.0f), glm::vec3(0, 1, 0));
		model_matrix = glm::rotate(model_matrix, glm::radians(45.0f), glm::vec3(0, 0, 1));
		model_matrix = glm::scale(model_matrix, glm::vec3(0.1, 6, 1.75));
		CurrentShader->SetModel(model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, PalmLeafDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Palm tree leaf 2
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, glm::vec3(-1.0f, 4.75f, -1.0));
		model_matrix = glm::rotate(model_matrix, glm::radians(135.0f), glm::vec3(0, 1, 0));
		model_matrix = glm::rotate(model_matrix, glm::radians(45.0f), glm::vec3(0, 0, 1));
		model_matrix = glm::scale(model_matrix, glm::vec3(0.1, 6, 1.75));
		CurrentShader->SetModel(model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, PalmLeafDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Palm tree leaf 3
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, glm::vec3(1.75f, 4.75f, 1.75));
		model_matrix = glm::rotate(model_matrix, glm::radians(-45.0f), glm::vec3(0, 1, 0));
		model_matrix = glm::rotate(model_matrix, glm::radians(45.0f), glm::vec3(0, 0, 1));
		model_matrix = glm::scale(model_matrix, glm::vec3(0.1, 6, 1.75));
		CurrentShader->SetModel(model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, PalmLeafDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Island for lighthouse 
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, glm::vec3(-2.0f, -2.55f, -15.0f));
		model_matrix = glm::scale(model_matrix, glm::vec3(3.25));
		CurrentShader->SetModel(model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, RockDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Lighthouse Top
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, glm::vec3(-2.0f, 1.5f, -15.0f));
		model_matrix = glm::scale(model_matrix, glm::vec3(1, 1, 1));
		CurrentShader->SetModel(model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, LighthouseDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Lighthouse Middle
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, glm::vec3(-2.0f, 0.5f, -15.0f));
		model_matrix = glm::scale(model_matrix, glm::vec3(1.0));
		CurrentShader->SetModel(model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, LighthouseDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Lighthouse Bottom
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, glm::vec3(-2.0f, -0.5f, -15.0f));
		model_matrix = glm::scale(model_matrix, glm::vec3(1, 1, 1));
		CurrentShader->SetModel(model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, LighthouseDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Lighthouse Lamp
		glm::vec3 LighthousePosition(-2.0f, 2.5f, -15.0f);
		model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, LighthousePosition);
		model_matrix = glm::scale(model_matrix, glm::vec3(1.42));
		double speed_of_rotation = 250;
		model_matrix = glm::rotate(model_matrix, glm::radians(static_cast<float>(start_time * speed_of_rotation)), glm::vec3(0, 1, 0));
		CurrentShader->SetModel(model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, LighthouseLampDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, LighthouseLampSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);

		if (clouds_and_lighthouse_light_visibility)
		{
			// Removing lighthouse lights
			CurrentShader->SetUniform3f("uLighthouseLight1.Position", glm::vec3(-10));
			CurrentShader->SetUniform3f("uLighthouseLight1.Direction", glm::vec3(-20));
			CurrentShader->SetUniform3f("uLighthouseLight2.Position", glm::vec3(-10));
			CurrentShader->SetUniform3f("uLighthouseLight2.Direction", glm::vec3(-20));
			CurrentShader->SetUniform3f("uLightHousePointLight.Position", glm::vec3(-20));

			// Fixed size cloud
			model_matrix = glm::mat4(1.0f);
			model_matrix = glm::translate(model_matrix, glm::vec3(-7.0f, 5.0f, -20.0f));
			model_matrix = glm::rotate(model_matrix, glm::radians(static_cast<float>(start_time) * 15), glm::vec3(0.5, 1, 1));
			model_matrix = glm::scale(model_matrix, glm::vec3(3, 1, 1));
			CurrentShader->SetModel(model_matrix);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, CloudDiffuseTexture);
			glBindVertexArray(CubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

			// Changing size cloud
			model_matrix = glm::mat4(1.0f);
			model_matrix = glm::translate(model_matrix, glm::vec3(7.1f, 5.2f, -21.0f));
			model_matrix = glm::rotate(model_matrix, glm::radians(static_cast<float>(start_time) * 15), glm::vec3(0.5, 1, 1));
			model_matrix = glm::scale(model_matrix, glm::vec3(abs(sin(start_time)) * 2 + 2, abs(sin(start_time * 2)) * 2 + 2, abs(sin(start_time)) + 2));
			model_matrix = glm::rotate(model_matrix, glm::radians(static_cast<float>(start_time) * 15), glm::vec3(0.5, 1, 1));
			CurrentShader->SetModel(model_matrix);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, CloudDiffuseTexture);
			glBindVertexArray(CubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);
		}
		else
		{
			// Add lighthouse lights
			double light_house_light_rotation_speed = (pi / 180.0) * speed_of_rotation;
			CurrentShader->SetUniform3f("uLightHousePointLight.Position", LighthousePosition);

			CurrentShader->SetUniform3f("uLighthouseLight1.Position", LighthousePosition);
			CurrentShader->SetUniform3f("uLighthouseLight1.Direction", glm::vec3(sin(start_time * light_house_light_rotation_speed), -0.3, cos(start_time * light_house_light_rotation_speed)));

			CurrentShader->SetUniform3f("uLighthouseLight2.Position", LighthousePosition);
			CurrentShader->SetUniform3f("uLighthouseLight2.Direction", glm::vec3(sin(start_time * light_house_light_rotation_speed + pi), -0.3, cos(start_time * light_house_light_rotation_speed + pi)));
		}

		glBindVertexArray(0);
		glUseProgram(0);
		glfwSwapBuffers(Window);
		State.mDT = glfwGetTime() - start_time;
	}

	glfwTerminate();
	return 0;
}
