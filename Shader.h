#pragma once

#include <glad/glad.h>

#include <fstream>
#include <iostream>
#include <string>

//#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
	unsigned int id;
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);

	void use();
	bool used() const;

	void SetBool(const char* name, bool value) const;
	void SetInt(const char* name, int value) const;
	void SetFloat(const char* name, float value) const;
	void SetMat4(const char* name, const float* value);
	void SetVec3(const char* name, float x, float y, float z);
	void SetVec3(const char* name, const glm::vec3& vec);
	void SetVec4(const char* name, float x, float y, float z, float w);
	void SetVec4(const char* name, const glm::vec4& vec);
	void SetVec3Array(const char* name, float* values, unsigned int count);
private:
	std::string get_file_contents(const char* path);
};