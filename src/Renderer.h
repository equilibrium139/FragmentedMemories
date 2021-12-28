#ifndef RENDERER_H
#define RENDERER_H

#include "Scene.h"

class Renderer {
public: 
	Renderer();
	void Draw(Scene& scene, float dt, int windowWidth, int windowHeight);

	Shader shader = Shader("Shaders/anim.vert", "Shaders/anim.frag");
	Shader simpleShader = Shader("Shaders/default.vert", "Shaders/default.frag");
	static constexpr int maxPointLights = 25;
	static constexpr int maxSpotLights = 25;
	unsigned int projViewUBO;
	unsigned int lightsUBO;
};

#endif // RENDERER_H