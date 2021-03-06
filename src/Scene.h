#ifndef SCENE_H
#define SCENE_H

#include "AnimatedModel.h"
#include "Camera.h"
#include "Light.h"
#include <vector>

struct Scene {
	Scene();

	std::vector<PointLight> pointLights;
	std::vector<SpotLight> spotLights;
	DirectionalLight dirLight;
	std::vector<AnimatedModel> animatedModels;
	Camera camera = Camera(glm::vec3(0.0f, 1.5f, 3.0f));
};

#endif