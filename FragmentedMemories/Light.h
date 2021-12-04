#ifndef LIGHT_H
#define LIGHT_H

#include <cmath>
#include <glm/glm.hpp>

// Padding added to match std140 layout

struct PointLight {
	glm::vec3 ambient;
	float range;
	glm::vec3 diffuse;
	float pad0;
	glm::vec3 specular;
	float pad1;
	glm::vec3 position;
	float pad2;

	PointLight(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 position, float range)
		:ambient(ambient), diffuse(diffuse), specular(specular), position(position), range(range) {}
};

struct DirectionalLight {
	glm::vec3 ambient;
	float pad0;
	glm::vec3 diffuse;
	float pad1;
	glm::vec3 specular;
	float pad2;
	glm::vec3 direction;
	float pad3;

	DirectionalLight(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 direction)
		:ambient(ambient), diffuse(diffuse), specular(specular), direction(direction) {}
};

struct SpotLight {
	glm::vec3 ambient;
	float rMaxSquared;
	glm::vec3 diffuse;
	float cosineInnerAngleCutoff;
	glm::vec3 specular;
	float cosineOuterAngleCutoff;
	glm::vec3 position;
	float pad0;
	glm::vec3 direction;
	float pad1;

	SpotLight(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 position, glm::vec3 direction, 
		float range, float innerAngleCutoff, float outerAngleCutoff)
		:ambient(ambient), diffuse(diffuse), specular(specular), position(position), direction(direction), 
		rMaxSquared(range * range), cosineInnerAngleCutoff(std::cos(innerAngleCutoff)), cosineOuterAngleCutoff(std::cos(outerAngleCutoff)) {}
};

#endif
