#pragma once

#include <string>
#include <vector>

unsigned int LoadTexture(const char* fileName, const std::string& directory);
unsigned int LoadCubemap(const std::vector<std::string>& faces);
unsigned int LoadTexture(unsigned char* buffer, int length, const std::string& identifier);

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