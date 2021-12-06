#ifndef RENDERER_H
#define RENDERER_H

#include "Scene.h"

class Renderer {
public: 
	Renderer(unsigned int projViewUBO, unsigned int lightsUBO);
	void Draw(Scene& scene, float dt, int windowWidth, int windowHeight, int maxPointLights, int maxSpotLights);

	Shader shader = Shader("Shaders/anim.vert", "Shaders/anim.frag");
	unsigned int projViewUBO;
	unsigned int lightsUBO;
};

#endif // RENDERER_H