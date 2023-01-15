
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


#define _USE_MATH_DEFINES


float
Clamp(float x, float min, float max) {
	return x < min ? min : x > max ? max : x;
}

int WindowWidth = 1280;
int WindowHeight = 720;
const float TargetFPS = 60.0f;
const std::string WindowTitle = "Karibi";



struct Input {
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

struct EngineState {
	Input* mInput;
	Camera* mCamera;
	unsigned mShadingMode;
	bool mDrawDebugLines;
	float mDT;
};

static void
ErrorCallback(int error, const char* description) {
	std::cerr << "GLFW Error: " << description << std::endl;
}


static void
KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	EngineState* State = (EngineState*)glfwGetWindowUserPointer(window);
	Input* UserInput = State->mInput;
	bool IsDown = action == GLFW_PRESS || action == GLFW_REPEAT;
	switch (key) {
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



	case GLFW_KEY_L: {
		if (IsDown) {
			State->mDrawDebugLines ^= true; break;
		}
	} break;

	case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
	}
}

static void
FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
	WindowWidth = width;
	WindowHeight = height;
	glViewport(0, 0, width, height);
}


static void
HandleInput(EngineState* state) {
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


static void
DrawSea(unsigned vao, const Shader& shader, unsigned diffuse, unsigned specular) {
	glUseProgram(shader.GetId());
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuse);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specular);
	float Size = 4.0f;
	int seaSize = 10;
	for (int i = -seaSize; i < seaSize; ++i) {
		for (int j = -seaSize; j < seaSize; ++j) {
			glm::mat4 Model(1.0f);
			Model = glm::translate(Model, glm::vec3(i * Size, (abs(sin(glfwGetTime() * 2))) - Size * 1.2, j * Size));
			Model = glm::scale(Model, glm::vec3(Size, Size, Size));
			shader.SetModel(Model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}


}

int main() {
	GLFWwindow* Window = 0;
	if (!glfwInit()) {
		std::cerr << "Failed to init glfw" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	Window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle.c_str(), 0, 0);
	if (!Window) {
		std::cerr << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(Window);

	GLenum GlewError = glewInit();
	if (GlewError != GLEW_OK) {
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



	std::vector<float> CubeVertices = {
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




	// NOTE(Jovan): Phong shader with material and texture support
	Shader PhongShaderMaterialTexture("shaders/basic.vert", "shaders/phong_material_texture.frag");
	glUseProgram(PhongShaderMaterialTexture.GetId());

	// Light from the sky 
	float light_from_sky = 0.7f;
	PhongShaderMaterialTexture.SetUniform3f("uDirLight.Direction", glm::vec3(0.0f));
	PhongShaderMaterialTexture.SetUniform3f("uDirLight.Ka", glm::vec3(light_from_sky));
	PhongShaderMaterialTexture.SetUniform3f("uDirLight.Kd", glm::vec3(light_from_sky));
	PhongShaderMaterialTexture.SetUniform3f("uDirLight.Ks", glm::vec3(light_from_sky));

	// Default for point light
	PhongShaderMaterialTexture.SetUniform3f("uPointLight.Ka", glm::vec3(1.00, 0.97, 0.00));
	PhongShaderMaterialTexture.SetUniform3f("uPointLight.Kd", glm::vec3(1.00, 0.97, 0.00));
	PhongShaderMaterialTexture.SetUniform3f("uPointLight.Ks", glm::vec3(1.0f));
	PhongShaderMaterialTexture.SetUniform1f("uPointLight.Kc", 1.0f);
	PhongShaderMaterialTexture.SetUniform1f("uPointLight.Kl", 0.01f);
	PhongShaderMaterialTexture.SetUniform1f("uPointLight.Kq", 0.005f);

	//First light from the Lighthouse 
	PhongShaderMaterialTexture.SetUniform3f("uSpotlight.Position", glm::vec3(999));
	PhongShaderMaterialTexture.SetUniform3f("uSpotlight.Direction", glm::vec3(999));
	PhongShaderMaterialTexture.SetUniform3f("uSpotlight.Ka", glm::vec3(0));
	PhongShaderMaterialTexture.SetUniform3f("uSpotlight.Kd", glm::vec3(1));
	PhongShaderMaterialTexture.SetUniform3f("uSpotlight.Ks", glm::vec3(1.0f));
	PhongShaderMaterialTexture.SetUniform1f("uSpotlight.Kc", 1.0f);
	PhongShaderMaterialTexture.SetUniform1f("uSpotlight.Kl", 0.0092f);
	PhongShaderMaterialTexture.SetUniform1f("uSpotlight.Kq", 0.0002f);
	PhongShaderMaterialTexture.SetUniform1f("uSpotlight.InnerCutOff", glm::cos(glm::radians(25.5f)));
	PhongShaderMaterialTexture.SetUniform1f("uSpotlight.OuterCutOff", glm::cos(glm::radians(27.5f)));

	// Materials
	PhongShaderMaterialTexture.SetUniform1i("uMaterial.Kd", 0);
	PhongShaderMaterialTexture.SetUniform1i("uMaterial.Ks", 1);
	PhongShaderMaterialTexture.SetUniform1f("uMaterial.Shininess", 64);
	glUseProgram(0);

	glm::mat4 Projection = glm::perspective(45.0f, WindowWidth / (float)WindowHeight, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(FPSCamera.GetPosition(), FPSCamera.GetTarget(), FPSCamera.GetUp());
	glm::mat4 ModelMatrix(1.0f);

	float TargetFrameTime = 1.0f / TargetFPS;
	float StartTime = glfwGetTime();
	float EndTime = glfwGetTime();
	glClearColor(0.53, 0.81, 0.98, 1.0);

	unsigned SeaDiffuseTexture = Texture::LoadImageToTexture("res/sea_d.jpg");
	unsigned SeaSpecularTexture = Texture::LoadImageToTexture("res/sea_s.jpg");
	unsigned SunDiffuseTexture = Texture::LoadImageToTexture("res/sun.jpg");
	unsigned SandDiffuseTexture = Texture::LoadImageToTexture("res/sand.jpg");
	unsigned RockDiffuseTexture = Texture::LoadImageToTexture("res/rock.jpg");
	unsigned LighthouseDiffuseTexture = Texture::LoadImageToTexture("res/lighthouse.jpg");
	unsigned LighthouseLampDiffuseTexture = Texture::LoadImageToTexture("res/lighthouseLamp.jpg");
	unsigned CloudDiffuseTexture = Texture::LoadImageToTexture("res/cloud.jpg");
	unsigned PalmTreeDiffuseTexture = Texture::LoadImageToTexture("res/palmTree.jpg");
	unsigned PalmLeafDiffuseTexture = Texture::LoadImageToTexture("res/palmLeaf.jpg");
	unsigned CampfireDiffuseTexture = Texture::LoadImageToTexture("res/campfire.jpg");

	Shader* CurrentShader = &PhongShaderMaterialTexture;

	bool clouds_and_lighthouse_light_visibility = true;

	double PI = atan(1) * 4;
	float i = 1;
	while (!glfwWindowShouldClose(Window)) {
		glfwPollEvents();
		HandleInput(&State);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	

		Projection = glm::perspective(90.0f, WindowWidth / (float)WindowHeight, 0.1f, 100.0f);
		View = glm::lookAt(FPSCamera.GetPosition(), FPSCamera.GetTarget(), FPSCamera.GetUp());
		StartTime = glfwGetTime();
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

		// Sun
		glm::vec3 PointLightPosition(15, 15.0f, 0);
		CurrentShader->SetUniform3f("uPointLight.Position", PointLightPosition);
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, PointLightPosition);
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(7));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SunDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		//Sea
		DrawSea(CubeVAO, *CurrentShader, SeaDiffuseTexture, SeaSpecularTexture);

		// Small island (Far)
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(25.0f, -2.7f, 25.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(4));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SandDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Torch on small island (far)
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(25.0f, -0.7, 25.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, CampfireDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Small island (Near)
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-20.0f, -2.7f, -15.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(4));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SandDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Torch on small island (near)
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-20.0f, -0.7f, -15.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, CampfireDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Big island
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -6.5f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(10));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, SandDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Torch on big island (near)
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(3.0f, -1.4f, 3.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, CampfireDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);


		// Palm tree
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 1.5f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1, 10, 1));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, PalmTreeDiffuseTexture);
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
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Island for lighthouse 
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-2.0f, -2.5f, -15.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, RockDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Lighthouse Top
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-2.0f, 1.5f, -15.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1, 1, 1));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, LighthouseDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Lighthouse Middle
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-2.0f, 0.5f, -15.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, LighthouseDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Lighthouse Bottom
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-2.0f, -0.5f, -15.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1, 1, 1));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, LighthouseDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		// Lighthouse Lamp

		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-2.0f, 2.5f, -15.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.42));

		float time = glfwGetTime();
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(time * 30), glm::vec3(0, 1, 0));
		CurrentShader->SetModel(ModelMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, LighthouseLampDiffuseTexture);
		glBindVertexArray(CubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

		if (clouds_and_lighthouse_light_visibility)
		{

			double lightHouseLightRotationSpeed = (PI / 180.0) * 30;
			CurrentShader->SetUniform3f("uSpotlight.Position", glm::vec3(-2.0f, 2.7f, -15.0f));
			CurrentShader->SetUniform3f("uSpotlight.Direction", glm::vec3(sin(time * lightHouseLightRotationSpeed), -0.3, cos(time * lightHouseLightRotationSpeed)));

			// Fixed size cloud
			ModelMatrix = glm::mat4(1.0f);
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-7.0f, 5.0f, -20.0f));
			ModelMatrix = glm::rotate(ModelMatrix, glm::radians((float)glfwGetTime() * 60), glm::vec3(0.5, 1, 1));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3, 1, 1));
			CurrentShader->SetModel(ModelMatrix);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, CloudDiffuseTexture);
			glBindVertexArray(CubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);

			// Changing size cloud
			ModelMatrix = glm::mat4(1.0f);
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(7.1f, 5.2f, -21.0f));
			ModelMatrix = glm::rotate(ModelMatrix, glm::radians((float)glfwGetTime() * 30), glm::vec3(0.5, 1, 1));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(abs(sin(glfwGetTime())) * 2 + 2, abs(sin(glfwGetTime() * 2)) * 2 + 2, abs(sin(glfwGetTime())) + 2));
			ModelMatrix = glm::rotate(ModelMatrix, glm::radians((float)glfwGetTime() * 30), glm::vec3(0.5, 1, 1));
			CurrentShader->SetModel(ModelMatrix);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, CloudDiffuseTexture);
			glBindVertexArray(CubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, CubeVertices.size() / 8);
		}
		else
		{

			CurrentShader->SetUniform3f("uSpotlight.Position", glm::vec3(-10));
			CurrentShader->SetUniform3f("uSpotlight.Direction", glm::vec3(-20));
		}


		glBindVertexArray(0);
		glUseProgram(0);
		glfwSwapBuffers(Window);

		EndTime = glfwGetTime();
		float WorkTime = EndTime - StartTime;
		if (WorkTime < TargetFrameTime) {
			int DeltaMS = (int)((TargetFrameTime - WorkTime) * 1000.0f);
			std::this_thread::sleep_for(std::chrono::milliseconds(DeltaMS));
			EndTime = glfwGetTime();
		}
		State.mDT = EndTime - StartTime;
	}

	glfwTerminate();
	return 0;
}
