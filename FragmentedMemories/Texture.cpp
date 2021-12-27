#include "Texture.h"

#include "stb_image.h"

#include <glad/glad.h>
#include <iostream>

#include <unordered_map>

static std::unordered_map<std::string, unsigned int> textures;

static unsigned int TextureFromFile(const char* path)
{
	int width, height, nrComponents;

	auto* data = stbi_load(path, &width, &height, &nrComponents, 0);

	GLuint id;
	glGenTextures(1, &id);

	if (data)
	{
		GLenum format;
		if (nrComponents == 1) format = GL_RED;
		else if (nrComponents == 3) format = GL_RGB;
		else if (nrComponents == 4) format = GL_RGBA;
		else {
			std::cout << "TextureFromFile::Unsupported number of components: " << nrComponents << " from file " << path << '\n';
		}

		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		std::cout << "TextureFromFile::Failed to load texture from file '" << path << "'.\n";
	}

	stbi_image_free(data);
	return id;
}


static unsigned int TextureFromMemory(unsigned char* buffer, int length)
{
	int width, height, nrComponents;

	auto* data = stbi_load_from_memory(buffer, length, &width, &height, &nrComponents, 0);

	GLuint id;
	glGenTextures(1, &id);

	if (data)
	{
		GLenum format;
		if (nrComponents == 1) format = GL_RED;
		else if (nrComponents == 3) format = GL_RGB;
		else if (nrComponents == 4) format = GL_RGBA;
		else {
			std::cout << "TextureFromMemory::Unsupported number of components: " << nrComponents << '\n';
		}

		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		std::cout << "TextureFromMemory::Failed to load texture from memory\n";
	}

	stbi_image_free(data);
	return id;
}

// Warning: Don't manually free textures that are loaded from this function.
// Otherwise they will still get returned if looked up rather than reloaded.
// TODO: add freeTexture function if needed.
unsigned int LoadTexture(const char* fileName, const std::string& directory)
{
	std::string path = directory + "/" + fileName;
	auto iter = textures.find(path);
	if (iter != textures.end())
	{
		return iter->second;
	}
	unsigned int id = TextureFromFile(path.c_str());
	textures[path] = id;
	return id;
}

unsigned int LoadTexture(unsigned char* buffer, int length, const std::string& identifier)
{
	auto iter = textures.find(identifier);
	if (iter != textures.end())
	{
		return iter->second;
	}
	unsigned int id = TextureFromMemory(buffer, length);
	textures[identifier] = id;
	return id;
}

unsigned int LoadCubemap(const std::vector<std::string>& faces)
{
	unsigned int ID;
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++) {
		auto* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else {
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << "\n";
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return ID;
}
