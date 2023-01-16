#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <thread>
#include "shader.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "texture.hpp"

int WindowWidth = 1280;
int WindowHeight = 720;
const float TargetFPS = 144.0f;
const std::string WindowTitle = "Karibi Kontrolna Tacka 02";

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
	float mDT;
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
	WindowWidth = width;
	WindowHeight = height;
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

static void DrawSea(unsigned vao, const Shader& shader, unsigned diffuse, unsigned specular)
{
	glUseProgram(shader.GetId());
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuse);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specular);
	float Size = 4.0f;
	int seaSize = 10;
	float time = glfwGetTime();
	for (int i = -seaSize; i < seaSize; ++i)
	{
		for (int j = -seaSize; j < seaSize; ++j)
		{
			glm::mat4 Model(1.0f);
			Model = glm::translate(Model, glm::vec3(i * Size, (abs(sin(time * 2))) - Size * 1.2, j * Size));
			Model = glm::scale(Model, glm::vec3(Size, Size, Size));
			shader.SetModel(Model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}
}

int main()
{
	GLFWwindow* Window = 0;
	if (!glfwInit())
	{
		std::cerr << "Failed to init glfw" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	Window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle.c_str(), 0, 0);
	if (!Window)
	{
		std::cerr << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(Window);
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	glfwSetWindowPos(Window, (mode->width - WindowWidth) / 2, (mode->height - WindowHeight) / 2);

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

	glViewport(0.0f, 0.0f, WindowWidth, WindowHeight);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	Model Woman("res/Woman/091_W_Aya_100K.obj");
	if (!Woman.Load()) {
		std::cerr << "Failed to load model\n";
		glfwTerminate();
		return -1;
	}
	Model Shark("res/Shark/SHARK.obj");
	if (!Shark.Load()) {
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
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
	float far_far_away_light = 1.0f;
	PhongShaderMaterialTexture.SetUniform3f("uDirLight.Direction", glm::vec3(0.0f));
	PhongShaderMaterialTexture.SetUniform3f("uDirLight.Ka", glm::vec3(far_far_away_light));
	PhongShaderMaterialTexture.SetUniform3f("uDirLight.Kd", glm::vec3(far_far_away_light));
	PhongShaderMaterialTexture.SetUniform3f("uDirLight.Ks", glm::vec3(far_far_away_light));

	// Default for point light (Sun)
	PhongShaderMaterialTexture.SetUniform3f("uSunLight.Ka", glm::vec3(1.00, 0.97, 0.00));
	PhongShaderMaterialTexture.SetUniform3f("uSunLight.Kd", glm::vec3(1.00, 0.97, 0.00));
	PhongShaderMaterialTexture.SetUniform3f("uSunLight.Ks", glm::vec3(1.0f));
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
	PhongShaderMaterialTexture.SetUniform3f("uLighthouseLight2.Ks", glm::vec3(1.0f));
	PhongShaderMaterialTexture.SetUniform1f("uLighthouseLight2.Kc", 1.0f);
	PhongShaderMaterialTexture.SetUniform1f("uLighthouseLight2.Kl", 0.0002f);
	PhongShaderMaterialTexture.SetUniform1f("uLighthouseLight2.Kq", 0.0002f);
	PhongShaderMaterialTexture.SetUniform1f("uLighthouseLight2.InnerCutOff", glm::cos(glm::radians(10.0f)));
	PhongShaderMaterialTexture.SetUniform1f("uLighthouseLight2.OuterCutOff", glm::cos(glm::radians(35.0f)));

	// Materials
	PhongShaderMaterialTexture.SetUniform1i("uMaterial.Kd", 0);
	PhongShaderMaterialTexture.SetUniform1i("uMaterial.Ks", 1);
	PhongShaderMaterialTexture.SetUniform1f("uMaterial.Shininess", 64);

	glUseProgram(0);

	glm::mat4 Projection = glm::perspective(90.0f, WindowWidth / (float)WindowHeight, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(FPSCamera.GetPosition(), FPSCamera.GetTarget(), FPSCamera.GetUp());
	glm::mat4 ModelMatrix(1.0f);

	float TargetFrameTime = 1.0f / TargetFPS;
	float StartTime = glfwGetTime();
	float EndTime = glfwGetTime();


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

	unsigned SeaSpecularTexture = Texture::LoadImageToTexture("res/sea_s.jpg");
	unsigned LighthouseLampSpecularTexture = Texture::LoadImageToTexture("res/lighthouseLamp_s.jpg");
	unsigned BlackSpecularTexture = Texture::LoadImageToTexture("res/black.jpg");

	Shader* CurrentShader = &PhongShaderMaterialTexture;
	bool clouds_and_lighthouse_light_visibility = true;
	bool IsDay = true;
	double PI = atan(1) * 4;
	glClearColor(0.53, 0.81, 0.98, 1.0);

	while (!glfwWindowShouldClose(Window)) {
		StartTime = glfwGetTime();
		glfwPollEvents();
		HandleInput(&State);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Projection = glm::perspective(90.0f, WindowWidth / (float)WindowHeight, 0.1f, 100.0f);
		View = glm::lookAt(FPSCamera.GetPosition(), FPSCamera.GetTarget(), FPSCamera.GetUp());
		glUseProgram(CurrentShader->GetId());
		CurrentShader->SetProjection(Projection);
		CurrentShader->SetView(View);
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
			IsDay = false;
		}
		if (glfwGetKey(Window, GLFW_KEY_L) == GLFW_PRESS)
		{
			IsDay = true;
		}

		if (IsDay)
		{
			glClearColor(0.53, 0.81, 0.98, 1.0);

			far_far_away_light = 1;
			CurrentShader->SetUniform3f("uDirLight.Ka", glm::vec3(far_far_away_light));
			CurrentShader->SetUniform3f("uDirLight.Kd", glm::vec3(far_far_away_light));
			CurrentShader->SetUniform3f("uDirLight.Ks", glm::vec3(far_far_away_light));

			// Sun
			glm::vec3 PointLightPositionSun(-25, 15.0f, 15);
			CurrentShader->SetUniform3f("uSunLight.Position", PointLightPositionSun);
			ModelMatrix = glm::mat4(1.0f);
			ModelMatrix = glm::translate(ModelMatrix, PointLightPositionSun);
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(7));
			CurrentShader->SetModel(ModelMatrix);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, SunDiffuseTexture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
			glBindVertexArray(CubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);
		}

		if (!IsDay)
		{
			glClearColor(0.0, 0.0, 0.0, 1.0);

			far_far_away_light = 0;
			CurrentShader->SetUniform3f("uDirLight.Ka", glm::vec3(far_far_away_light));
			CurrentShader->SetUniform3f("uDirLight.Kd", glm::vec3(far_far_away_light));
			CurrentShader->SetUniform3f("uDirLight.Ks", glm::vec3(far_far_away_light));
			CurrentShader->SetUniform3f("uSunLight.Position", glm::vec3(-999));

			// Sharks
			int numberOfSharks = 4;
			for (int i = 0; i < numberOfSharks; i++)
			{
				float AngleOfShark = (2 * PI / numberOfSharks) * i;
				float DistanceFromIsland = 10;
				ModelMatrix = glm::mat4(1.0f);
				ModelMatrix = glm::translate(ModelMatrix, glm::vec3(DistanceFromIsland * sin(AngleOfShark + glfwGetTime()), -3.0f, DistanceFromIsland * cos(AngleOfShark + glfwGetTime())));
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.5));
				ModelMatrix = glm::rotate(ModelMatrix, glm::radians((float)glfwGetTime() * 100), glm::vec3(0, 1, 0));
				ModelMatrix = glm::rotate(ModelMatrix, glm::radians(-45.0f), glm::vec3(0, 0, 1));
				CurrentShader->SetModel(ModelMatrix);
				Shark.Render();
			}

		}

		// Sea
		DrawSea(CubeVAO, *CurrentShader, SeaDiffuseTexture, SeaSpecularTexture);

		// Small island (Far)
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(25.0f, -2.7f, 25.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(4));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SandDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Torch on small island (Far)
		glm::vec3 PointLightPositionTorch1(25.0f, -0.7, 25.0f);
		CurrentShader->SetUniform3f("uTorchLight1.Position", PointLightPositionTorch1);
		CurrentShader->SetUniform1f("uTorchLight1.Kc", 0.1 / abs(sin(glfwGetTime() * 2)));
		CurrentShader->SetUniform1f("uTorchLight1.Kl", 0.1 / abs(sin(glfwGetTime() * 3)));
		CurrentShader->SetUniform1f("uTorchLight1.Kq", 1.0 / abs(sin(glfwGetTime() * 5)));
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, PointLightPositionTorch1);
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, CampfireDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Small island (Near)
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-20.0f, -2.7f, -15.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(4));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SandDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Torch on small island (Near)
		glm::vec3 PointLightPositionSunTorch2(-20.0f, -0.7f, -15.0f);
		CurrentShader->SetUniform3f("uTorchLight2.Position", PointLightPositionSunTorch2);
		CurrentShader->SetUniform1f("uTorchLight2.Kc", 0.1 / abs(sin(glfwGetTime() * 2)));
		CurrentShader->SetUniform1f("uTorchLight2.Kl", 0.091 / abs(sin(glfwGetTime() * 3)));
		CurrentShader->SetUniform1f("uTorchLight2.Kq", 0.1 / abs(sin(glfwGetTime() * 5)));
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, PointLightPositionSunTorch2);
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, CampfireDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Big island
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -6.5f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(10));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SandDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Torch on big island
		glm::vec3 PointLightPositionSunTorch3(3.5f, -1.4f, 3.5f);
		CurrentShader->SetUniform3f("uTorchLight3.Position", PointLightPositionSunTorch3);
		CurrentShader->SetUniform1f("uTorchLight3.Kc", 1 / abs(sin(glfwGetTime() * 2)));
		CurrentShader->SetUniform1f("uTorchLight3.Kl", 0.1 / abs(sin(glfwGetTime() * 3)));
		CurrentShader->SetUniform1f("uTorchLight3.Kq", 0.1 / abs(sin(glfwGetTime() * 5)));
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, PointLightPositionSunTorch3);
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, CampfireDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Model on island (Woman)
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-4.5f, -2.25f, -4.5f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.002));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(155.0f), glm::vec3(0, 1, 0));
		CurrentShader->SetModel(ModelMatrix);
		Woman.Render();

		// Palm tree
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 1.5f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1, 10, 1));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, PalmTreeDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Palm tree top leaf
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 6.0f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(45.0f), glm::vec3(0, 1, 0));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, PalmLeafDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Palm tree leaf 1
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(1.5f, 4.75f, -1.5f));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(45.0f), glm::vec3(0, 1, 0));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(45.0f), glm::vec3(0, 0, 1));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.1, 6, 1.75));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, PalmLeafDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Palm tree leaf 2
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-1.0f, 4.75f, -1.0));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(135.0f), glm::vec3(0, 1, 0));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(45.0f), glm::vec3(0, 0, 1));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.1, 6, 1.75));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, PalmLeafDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Palm tree leaf 3
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(1.75f, 4.75f, 1.75));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(-45.0f), glm::vec3(0, 1, 0));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(45.0f), glm::vec3(0, 0, 1));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.1, 6, 1.75));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, PalmLeafDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Island for lighthouse 
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-2.0f, -2.5f, -15.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, RockDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Lighthouse Top
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-2.0f, 1.5f, -15.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1, 1, 1));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, LighthouseDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Lighthouse Middle
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-2.0f, 0.5f, -15.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, LighthouseDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Lighthouse Bottom
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-2.0f, -0.5f, -15.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1, 1, 1));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, LighthouseDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Lighthouse Lamp
		glm::vec3 LighthousePosition(-2.0f, 2.5f, -15.0f);
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, LighthousePosition);
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.42));
		float time = glfwGetTime();
		float speedOfRotation = 170;
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(time * speedOfRotation), glm::vec3(0, 1, 0));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, LighthouseLampDiffuseTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, LighthouseLampSpecularTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		if (clouds_and_lighthouse_light_visibility)
		{
			// Removing lighthouse lights
			CurrentShader->SetUniform3f("uLighthouseLight1.Position", glm::vec3(-10));
			CurrentShader->SetUniform3f("uLighthouseLight1.Direction", glm::vec3(-20));

			CurrentShader->SetUniform3f("uLighthouseLight2.Position", glm::vec3(-10));
			CurrentShader->SetUniform3f("uLighthouseLight2.Direction", glm::vec3(-20));

			CurrentShader->SetUniform3f("uLightHousePointLight.Position", glm::vec3(-20));

			// Fixed size cloud
			ModelMatrix = glm::mat4(1.0f);
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-7.0f, 5.0f, -20.0f));
			ModelMatrix = glm::rotate(ModelMatrix, glm::radians((float)glfwGetTime() * 15), glm::vec3(0.5, 1, 1));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3, 1, 1));
			CurrentShader->SetModel(ModelMatrix);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, CloudDiffuseTexture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
			glBindVertexArray(CubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

			// Changing size cloud
			ModelMatrix = glm::mat4(1.0f);
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(7.1f, 5.2f, -21.0f));
			ModelMatrix = glm::rotate(ModelMatrix, glm::radians((float)glfwGetTime() * 15), glm::vec3(0.5, 1, 1));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(abs(sin(glfwGetTime())) * 2 + 2, abs(sin(glfwGetTime() * 2)) * 2 + 2, abs(sin(glfwGetTime())) + 2));
			ModelMatrix = glm::rotate(ModelMatrix, glm::radians((float)glfwGetTime() * 15), glm::vec3(0.5, 1, 1));
			CurrentShader->SetModel(ModelMatrix);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, CloudDiffuseTexture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, BlackSpecularTexture);
			glBindVertexArray(CubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);
		}
		else
		{
			// Removing lighthouse lights
			double lightHouseLightRotationSpeed = (PI / 180.0) * speedOfRotation;
			CurrentShader->SetUniform3f("uLightHousePointLight.Position", LighthousePosition);

			CurrentShader->SetUniform3f("uLighthouseLight1.Position", LighthousePosition);
			CurrentShader->SetUniform3f("uLighthouseLight1.Direction", glm::vec3(sin(time * lightHouseLightRotationSpeed), -0.3, cos(time * lightHouseLightRotationSpeed)));

			CurrentShader->SetUniform3f("uLighthouseLight2.Position", LighthousePosition);
			CurrentShader->SetUniform3f("uLighthouseLight2.Direction", glm::vec3(sin(time * lightHouseLightRotationSpeed + PI), -0.3, cos(time * lightHouseLightRotationSpeed + PI)));
		}

		glBindVertexArray(0);
		glUseProgram(0);
		glfwSwapBuffers(Window);

		EndTime = glfwGetTime();
		float WorkTime = EndTime - StartTime;
		if (WorkTime < TargetFrameTime)
		{
			int DeltaMS = (int)((TargetFrameTime - WorkTime) * 1000.0f);
			std::this_thread::sleep_for(std::chrono::milliseconds(DeltaMS));
			EndTime = glfwGetTime();
		}
		State.mDT = EndTime - StartTime;
	}

	glfwTerminate();
	return 0;
}
