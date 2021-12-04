#ifndef SCENE_H
#define SCENE_H

#include "AnimatedModel.h"
#include "Camera.h"
#include "Light.h"
#include <vector>

struct Scene {
	Scene();
	void Draw(float dt, float time);

	std::vector<PointLight> pointLights;
	std::vector<DirectionalLight> dirLights;
	std::vector<SpotLight> spotLights;
	std::vector<AnimatedModel> animatedModels;
	// Camera camera;
	Shader shader = Shader("Shaders/anim.vert", "Shaders/anim.frag");
	unsigned int projViewUBO, lightsUBO;
};

#endif 