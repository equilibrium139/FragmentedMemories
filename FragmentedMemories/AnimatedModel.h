#ifndef ANIMATED_MODEL_H

#include <assimp/scene.h>
#include <cstdint>
#include <glm/glm.hpp>
#include "Material.h"
#include "Shader.h"
#include <string>
#include <vector>


struct SkinnedVertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coords;
	glm::u8vec4 joint_indices;
	glm::vec4 joint_weights;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

struct SkinnedMesh
{
	unsigned int indices_begin, index_count;
	PhongMaterial material;
	void Draw(Shader& shader);
};

struct Joint
{
	glm::mat4x3 local_to_joint; // AKA "inverse bind matrix". Apparently 4x3 actually means 3 rows 4 columns in glm so this is fine
	int parent;
};

struct Skeleton
{
	std::vector<Joint> joints;
	std::vector<std::string> joint_names;
};

struct JointPose
{
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 scale;
};

struct SkeletonPose
{
	std::vector<JointPose> local_joint_poses; // relative to parent joint
	std::vector<glm::mat4> global_joint_poses; // relative to model
};

struct AnimationClip
{
	Skeleton* skeleton;
	std::vector<SkeletonPose> poses;
	float frames_per_second;
	unsigned int frame_count;
};

struct AnimatedModel
{
	Skeleton skeleton;
	std::vector<SkinnedVertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<SkinnedMesh> meshes;
	std::vector<JointPose> bind_pose;
	AnimationClip clip;
	std::string directory;
	AnimatedModel(const std::string& path);
	void Draw(Shader& shader, float dt);
private:
	void LoadAnimatedModel(const std::string& path);
	void CreateSkeleton(const aiScene* scene, const std::vector<aiNode*>& nodes, const std::vector<int>& parent);
	void CreateMeshes(const aiScene* scene);
	void CreateAnimationClip(const aiScene* scene);
	float time = 0.0f;
	unsigned int VAO, VBO, EBO;
};



#endif // !ANIMATED_MODEL_H
