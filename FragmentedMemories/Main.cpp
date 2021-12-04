#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <iostream>
#include <string>

#include "AnimatedModel.h"
#include "Camera.h"
#include "Shader.h"
#include "stb_image.h"
#include "Model.h"

int windowWidth = 800;
int windowHeight = 600;

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

float mouseX = 400.0f;
float mouseY = 300.0f;

Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		gCamera.ProcessKeyboard(CAM_FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		gCamera.ProcessKeyboard(CAM_BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		gCamera.ProcessKeyboard(CAM_LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		gCamera.ProcessKeyboard(CAM_RIGHT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

void cursorPosCallback(GLFWwindow* window, double x, double y)
{
	static bool firstMouse = true;
	if (firstMouse) {
		mouseX = (float)x;
		mouseY = (float)y;
		firstMouse = false;
	}

	float xoffset = x - mouseX;
	float yoffset = mouseY - y;
	mouseX = (float)x;
	mouseY = (float)y;

	gCamera.ProcessMouseMovement(xoffset, yoffset);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	gCamera.ProcessMouseScroll(yoffset);
}

int main()
{
	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW\n";
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Breakout", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD\n";
		return -1;
	}

	glViewport(0, 0, windowWidth, windowHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// glEnable(GL_CULL_FACE);

	Shader cubeShader{ "Shaders/anim.vert", "Shaders/anim.frag" };
	cubeShader.use();
	GLuint cubeShaderBlockIndex = glGetUniformBlockIndex(cubeShader.id, "Matrices");
	glUniformBlockBinding(cubeShader.id, cubeShaderBlockIndex, 0);
	
	GLuint projViewUBO;
	glGenBuffers(1, &projViewUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, projViewUBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);

	AnimatedModel bob("Models/Warrok/sitting_laughing.fbx");

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, projViewUBO);
	glEnable(GL_CULL_FACE);
	int i = 0;
	while (!glfwWindowShouldClose(window))
	{
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastFrameTime;
		lastFrameTime = currentTime;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		processInput(window);

		glm::mat4 model = glm::identity<glm::mat4>();
		model = glm::translate(model, glm::vec3(0, -2, 0));
		model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
		glm::mat4 view = gCamera.GetViewMatrix();
		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(view) * glm::mat3(model)));
		glm::mat4 proj = glm::perspective(glm::radians(gCamera.Zoom), float(windowWidth) / float(windowHeight), 0.001f, 1000.0f);

		glBindBuffer(GL_UNIFORM_BUFFER, projViewUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(proj));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

		cubeShader.use();
		cubeShader.SetMat4("model", glm::value_ptr(model));
		glUniformMatrix3fv(glGetUniformLocation(cubeShader.id, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
		
		bob.Draw(cubeShader, deltaTime);
		i++;
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}