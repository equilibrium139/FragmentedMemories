#ifndef PLANE_H
#define PLANE_H


#include <glm/glm.hpp>
#include <array>

#include "Shader.h"	
#include "Texture.h"

class Plane
{
public:
	struct Vertex 
	{
		glm::vec3 position;
		glm::vec2 texCoords;
	};

	Plane(std::string texturePath)
	{
		std::array<Vertex, 4> vertices;
		vertices[0] = { glm::vec3{-1.0f, -1.0f, 0.0f}, glm::vec2{0.0f, 0.0f} };
		vertices[1] = { glm::vec3{1.0f, -1.0f, 0.0f}, glm::vec2{1.0f, 0.0f} };
		vertices[2] = { glm::vec3{1.0f, 1.0f, 0.0f}, glm::vec2{1.0f, 1.0f} };
		vertices[3] = { glm::vec3{-1.0f, 1.0f, 0.0f}, glm::vec2{0.0f, 1.0f} };

		std::array<unsigned int, 6> indices = { 0, 1, 2, 0, 2, 3 };
	
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

		texture.id = LoadTexture(texturePath.c_str(), "Textures");
	}

	void Draw(Shader& shader)
	{
		shader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture.id);
		shader.SetInt("ourTexture", 0);

		glBindVertexArray(VAO);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
private:
	Texture texture;
	unsigned int VAO, VBO, EBO;
};

#endif // !PLANE_H
