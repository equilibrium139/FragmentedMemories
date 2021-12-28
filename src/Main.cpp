#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include "Game.h"
#include "Renderer.h"
#include "Scene.h"

int windowWidth = 1200;
int windowHeight = 800;

void framebufferSizeCallback(GLFWwindow* window, int width, int height) 
{
	glViewport(0, 0, width, height);
	windowWidth = width;
	windowHeight = height;
}

void ProcessInput(GameInput& outInput, GLFWwindow* window) 
{
	static bool firstPoll = true;
	auto prevCursorX = outInput.cursorX;
	auto prevCursorY = outInput.cursorY;
	glfwGetCursorPos(window, &outInput.cursorX, &outInput.cursorY);
	outInput.windowWidth = windowWidth;
	outInput.windowHeight = windowHeight;
	if (firstPoll) 
	{
		outInput.cursorDeltaX = 0.0;
		outInput.cursorDeltaY = 0.0;
		firstPoll = false;
	}
	else 
	{
		outInput.cursorDeltaX = outInput.cursorX - prevCursorX;
		outInput.cursorDeltaY = outInput.cursorY - prevCursorY;
	}

	outInput.moveUp = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
	outInput.moveDown = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
	outInput.moveLeft = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
	outInput.moveRight = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
	outInput.start = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	GameInput gameInput;
	Game game;

	float deltaTime = 0.0f;
	float lastFrameTime = 0.0f;

	while (!glfwWindowShouldClose(window)) {
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastFrameTime;
		lastFrameTime = currentTime;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ProcessInput(gameInput, window);
		auto gameOutput = game.UpdateAndRender(gameInput, deltaTime, currentTime);
		if (gameOutput.quit)
		{
			break;
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}