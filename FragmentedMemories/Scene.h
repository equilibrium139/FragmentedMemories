#ifndef SCENE_H
#define SCENE_H

#include "AnimatedModel.h"
#include "Camera.h"
#include "Light.h"
#include <vector>

struct Scene {
	Scene();
	void Update(float dt, float time);

	std::vector<PointLight> pointLights;
	std::vector<SpotLight> spotLights;
	DirectionalLight dirLight;
	std::vector<AnimatedModel> animatedModels;
	Camera camera;
};

#endif