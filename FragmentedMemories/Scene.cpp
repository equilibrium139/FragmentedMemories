#include "Scene.h"

#include <cmath>

constexpr float twoPI = 2 * 3.14f;

Scene::Scene()
	:dirLight(glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
	surface(
		[](float u, float v) {return 3 * std::cos(u * twoPI) + std::cos(v * twoPI) * std::cos(u * twoPI); }, // x
		[](float u, float v) {return 3 * std::sin(u * twoPI) + std::cos(v * twoPI) * std::sin(u * twoPI); }, // y
		[](float u, float v) {return std::sin(v * twoPI); },													// z
		[](float u, float v) {return (-std::cos(v * twoPI) - 3) * std::sin(u * twoPI); },					// dx/du
		[](float u, float v) {return -std::cos(u * twoPI) * std::sin(v * twoPI); },							// dx/dv
		[](float u, float v) {return (std::cos(v * twoPI) + 3) * std::cos(u * twoPI); },					// dy/du
		[](float u, float v) {return -std::sin(u * twoPI) * std::sin(v * twoPI); },							// dy/dv
		[](float u, float v) {return 0; },																	// dz/du
		[](float u, float v) {return std::cos(v * twoPI); },												// dz/dv
		"param_texture.jpg"),
	plane("param_texture.jpg")
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

	animatedModels.emplace_back("Models/Warrok/sitting_laughing.fbx");
}