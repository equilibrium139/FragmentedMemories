#ifndef ANIMATED_MODEL_H
#define ANIMATED_MODEL_H

#include <cstdint>
#include <glm/glm.hpp>
#include "Material.h"
#include "Shader.h"
#include <string>
#include <memory>

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

struct Mesh
{
	unsigned int indices_begin, indices_end;
	unsigned int material_index;
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
	std::vector<JointPose> joint_poses; // relative to parent joint
	// std::vector<glm::mat4> global_joint_poses; // relative to model
};

struct AnimationClip
{
	//Skeleton* skeleton;
	std::vector<SkeletonPose> poses;
	float frames_per_second;
	unsigned int frame_count;
	bool loops;
};

enum class VertexFlags : std::uint32_t
{
	DEFAULT = 0, // vec3 position vec3 normal vec2 uv
	HAS_TANGENT = 1, // vec3 tangent
	HAS_JOINT_DATA = 2, // uint32 joint indices vec4 joint weights
};

inline VertexFlags operator | (VertexFlags lhs, VertexFlags rhs)
{
	using T = std::underlying_type_t<VertexFlags>;
	return (VertexFlags)((T)lhs | (T)rhs);
}

inline VertexFlags& operator |= (VertexFlags& lhs, VertexFlags rhs)
{
	lhs = lhs | rhs;
	return lhs;
}

inline bool HasFlag(VertexFlags flags, VertexFlags flag_to_check)
{
	return (std::underlying_type_t<VertexFlags>)(flags | flag_to_check) != 0;
}

struct ModelFile
{
	struct Header
	{
		std::uint32_t magic_number = 'ldom';
		std::uint32_t num_meshes;
		std::uint32_t num_vertices;
		std::uint32_t num_indices;
		std::uint32_t num_materials;
		VertexFlags vertex_flags = VertexFlags::DEFAULT;
		// add padding if needed
	};
	Header header;
	std::unique_ptr<Mesh[]> meshes;
	std::unique_ptr<std::uint8_t[]> vertex_buffer;
	std::unique_ptr<unsigned int[]> indices;
	std::unique_ptr<PhongMaterial[]> materials;
	std::string name;
};

struct SkeletonFile
{
	struct Header
	{
		std::uint32_t magic_number = 'ntks';
		std::uint32_t num_joints;
	};
	Header header;
	std::unique_ptr<Joint[]> joints;
	std::unique_ptr<std::string[]> joint_names;
};

struct AnimationClipFile
{
	struct Header
	{
		using bool32 = std::uint32_t; // for padding purposes
		std::uint32_t magic_number = 'pilc';
		std::uint32_t frame_count;
		float frames_per_second;
		bool32 loops = false; // fix later
		// add padding if needed
	};
	Header header;
	std::unique_ptr<SkeletonPose[]> skeleton_poses; // number of poses = frame_count + 1 or frame_count if loops
	std::string name;
};

struct AnimatedModel
{
	std::vector<Mesh> meshes;
	std::vector<PhongMaterial> materials;
	Skeleton skeleton;
	AnimationClip clip;
	AnimatedModel(const std::string& path);
	void Draw(Shader& shader, float dt);
private:
	void LoadAnimatedModel(const std::string& path);
	float time = 0.0f;
	unsigned int VAO, VBO, EBO;
};

#endif // !ANIMATED_MODEL_H
