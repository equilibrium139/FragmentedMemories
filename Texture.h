#pragma once

#include <string>
#include <vector>

unsigned int loadTexture(const char* fileName, const std::string& directory);
unsigned int loadCubemap(const std::vector<std::string>& faces);

enum class TextureType
{
	Diffuse,
	Specular,
};

struct Texture
{
	unsigned int id;
	TextureType type;
};