#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include "Renderer.h"
#include "Scene.h"

int windowWidth = 800;
int windowHeight = 600;

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

float mouseX = 400.0f;
float mouseY = 300.0f;

Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));

void processInput(GLFWwindow* window) {
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

void cursorPosCallback(GLFWwindow* window, double x, double y) {
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

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	gCamera.ProcessMouseScroll(yoffset);
}

int main() {
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW\n";
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window =
		glfwCreateWindow(windowWidth, windowHeight, "Breakout", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
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
	glEnable(GL_CULL_FACE);

	constexpr int maxPointLights = 25;
	constexpr int maxSpotLights = 25;

	GLuint projViewUBO;
	glGenBuffers(1, &projViewUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, projViewUBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, projViewUBO);

	GLuint lightsUBO;
	glGenBuffers(1, &lightsUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, lightsUBO);
	glBufferData(GL_UNIFORM_BUFFER, maxPointLights * sizeof(PointLight) + maxSpotLights * sizeof(SpotLight) + sizeof(DirectionalLight) + 2 * sizeof(int), NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightsUBO);

	Scene scene;
	Renderer renderer = Renderer(projViewUBO, lightsUBO);

	while (!glfwWindowShouldClose(window)) {
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastFrameTime;
		lastFrameTime = currentTime;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		processInput(window);

		renderer.Draw(scene, deltaTime, windowWidth, windowHeight, maxPointLights, maxSpotLights);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}