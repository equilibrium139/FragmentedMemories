#include "Scene.h"

#include <cmath>

constexpr float twoPI = 2 * 3.14f;

Scene::Scene()
	:dirLight(glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
{
	spotLights.emplace_back(
		glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(5.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f), 20, glm::radians(10.0f),
		glm::radians(20.0f));

	spotLights.emplace_back(
		glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 5.0f),
		glm::vec3(0.0f, 0.0f, -1.0f), 20, glm::radians(10.0f),
		glm::radians(20.0f));

	animatedModels.emplace_back("Models/Warrok2/Warrok.model");
}