#include "Scene.h"

extern int windowWidth, windowHeight;

Scene::Scene()
	:dirLight(glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
{
	spotLights.emplace_back(
		glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -1.0f), 2, glm::radians(10.0f),
		glm::radians(20.0f));

	animatedModels.emplace_back("Models/Warrok/sitting_laughing.fbx");
}

void Scene::Update(float dt, float time)
{
	
}
