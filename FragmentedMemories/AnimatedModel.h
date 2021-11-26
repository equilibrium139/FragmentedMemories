#ifndef ANIMATED_MODEL_H

#include <assimp/scene.h>
#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include "Texture.h"
#include <vector>


struct SkinnedVertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coords;
	glm::u8vec4 joint_indices;
	glm::vec3 joint_weights; // last weight is stored implicitly as 1 - (x + y + z)
};

struct SkinnedMesh
{
	std::vector<SkinnedVertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

private:
	unsigned int VAO, VBO, EBO;
};

struct Joint
{
	const char* name;
	glm::mat4x3 inverse_bind_pose;
	std::uint8_t parent;
};

struct Skeleton
{
	std::vector<Joint> joints;
	std::vector<std::string> joint_names;
};

struct AnimatedModel
{
	std::vector<SkinnedMesh> meshes;
	Skeleton skeleton;
	std::string directory;
	AnimatedModel(const std::string& path);
private:
	void LoadAnimatedModel(const std::string& path);
	void ProcessNode(const aiNode* node, const aiScene* scene);
	SkinnedMesh ProcessMeshAndBones(const aiMesh* mesh);
};

#endif // !ANIMATED_MODEL_H
