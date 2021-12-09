#ifndef PARAMETRIC_SURFACE_H
#define PARAMETRIC_SURFACE_H

#include <glm/glm.hpp>
#include <functional>
#include <vector>

#include "Shader.h"	
#include "Texture.h"

using ParametricFunction = std::function<float(float, float)>;
using DerivativeFunction = ParametricFunction;

class ParametricSurface
{
public:
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoords;
	};

	ParametricSurface(ParametricFunction xFunction, ParametricFunction yFunction, ParametricFunction zFunction, 
					  DerivativeFunction dxdu, DerivativeFunction dxdv, DerivativeFunction dydu, DerivativeFunction dydv,
					  DerivativeFunction dzdu, DerivativeFunction dzdv, std::string texturePath, int resolution = 50)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		float step = 1.0f / (resolution - 1);
		for (int i = 0; i < resolution; i++)
		{
			float u = i * step;
			for (int j = 0; j < resolution; j++)
			{
				float v = j * step;
				glm::vec3 position(xFunction(u, v), yFunction(u, v), zFunction(u, v));
				glm::vec3 normal = glm::cross(
					glm::vec3(dxdu(u, v), dydu(u, v), dzdu(u, v)),
					glm::vec3(dxdv(u, v), dydv(u, v), dzdv(u, v))
					);
				glm::vec2 uv(u, v);
				vertices.push_back({ position, normal, uv });
			}
		}

		for (int i = 0; i < resolution - 1; i++)
		{
			for (int j = 0; j < resolution - 1; j++)
			{
				unsigned int bottomLeft = i * resolution + j;
				unsigned int bottomRight = bottomLeft + 1;
				unsigned int topLeft = (i + 1) * resolution + j;
				unsigned int topRight = topLeft + 1;

				indices.insert(indices.end(), { bottomLeft, bottomRight, topRight,
											    bottomLeft, topRight, topLeft });
			}
		}

		numIndices = indices.size();

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

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

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

		glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
	}

private:
	Texture texture;
	unsigned int numIndices;
	unsigned int VAO, VBO, EBO;
};

#endif // !PARAMETRIC_SURFACE_H
