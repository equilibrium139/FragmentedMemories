#pragma once

#include "Texture.h"
#include <glm/vec3.hpp>

struct PhongMaterial
{
	glm::vec3 diffuse_coefficient;
	glm::vec3 specular_coefficient;
	float shininess;

	Texture diffuse_map;
	Texture specular_map;
	Texture normal_map;
};