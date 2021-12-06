#ifndef MESH_H
#define MESH_H

#include <glm/vec3.hpp>

#include <string>
#include <vector>

#include "Shader.h"
#include "Texture.h"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
};

class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	std::string name;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, std::string name);
	void draw(Shader& shader);
private:
	unsigned int VAO, VBO, EBO;
	void setupMesh();
};

#endif // MESH_H