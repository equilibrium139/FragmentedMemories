#include "Mesh.h"

#include <cstddef>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, std::string name)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	this->name = name;

	setupMesh();
}

void Mesh::draw(Shader& shader)
{
	shader.use();

	int diffuseNr = 1;
	int specularNr = 1;

	const char* diffuseUniformName = "material.texture_diffuse";
	const char* specularUniformName = "material.texture_specular";

	// TODO: fix this
	/*for (unsigned int i = 0; i < textures.size(); i++)
	{
		Texture& texture = textures[i];
		std::string textureName;
		switch (texture.type)
		{
		case TextureType::Diffuse:
			textureName = diffuseUniformName + std::to_string(diffuseNr++);
			break;
		case TextureType::Specular:
			textureName = specularUniformName + std::to_string(specularNr++);
			break;
		default:
			break;
		}
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texture.id);
		shader.SetInt(textureName.c_str(), i);
	}*/

	/*if (diffuseNr == 1 && specularNr == 1)
	{
		GLuint tempTex;
		glGenTextures(1, &tempTex);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, tempTex);
		GLubyte data[4] = { 255, 0, 0, 255 };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		shader.SetInt("material.texture_diffuse1", 4);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glDeleteTextures(1, &tempTex);
		return;
	}*/

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}

void Mesh::setupMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

	// Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	// Normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	// Tex coord
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

	glBindVertexArray(0);
}
