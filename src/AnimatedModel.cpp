#include "AnimatedModel.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <utility>
#include <glm/glm.hpp>
#include "stb_image.h"

AnimatedModel::AnimatedModel(const std::string& path)
{
	LoadAnimatedModel(path);
}

// TODO: remove, probably not needed
static glm::mat4 ToMat4(glm::mat4x3& mat)
{
	glm::mat4 mat4;

	mat4[0][0] = mat[0][0];
	mat4[0][1] = mat[0][1];
	mat4[0][2] = mat[0][2];
	mat4[0][3] = 0.0f;

	mat4[1][0] = mat[1][0];
	mat4[1][1] = mat[1][1];
	mat4[1][2] = mat[1][2];
	mat4[1][3] = 0.0f;

	mat4[2][0] = mat[2][0];
	mat4[2][1] = mat[2][1];
	mat4[2][2] = mat[2][2];
	mat4[2][3] = 0.0f;

	mat4[3][0] = mat[3][0];
	mat4[3][1] = mat[3][1];
	mat4[3][2] = mat[3][2];
	mat4[3][3] = 1.0f;

	return mat4;
}

static inline glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t)
{
	return a * (1.0f - t) + b * t;
}

static SkeletonPose Interpolate(const SkeletonPose& a, const SkeletonPose& b, float t)
{
	const auto num_poses = a.joint_poses.size();
	SkeletonPose interpolated;
	interpolated.joint_poses.resize(num_poses);
	for (auto i = 0u; i < num_poses; i++)
	{
		auto& a_pose = a.joint_poses[i];
		auto& b_pose = b.joint_poses[i];
		auto& interpolated_joint_pose = interpolated.joint_poses[i];
		interpolated_joint_pose.translation = Lerp(a_pose.translation, b_pose.translation, t);
		interpolated_joint_pose.scale = Lerp(a_pose.scale, b_pose.scale, t);
		interpolated_joint_pose.rotation = glm::slerp(a_pose.rotation, b_pose.rotation, t);
	}
	return interpolated;
}

void AnimatedModel::Draw(Shader& shader, float dt)
{
	static float time = 0.0f;
	time += dt;
	float clip_duration = clip.frame_count / clip.frames_per_second;
	float pose_index = std::fmod(time, clip_duration) * clip.frames_per_second;
	auto& pose = clip.poses[pose_index];

	glm::mat4 local_mat = glm::identity<glm::mat4>();
	local_mat = glm::translate(local_mat, pose.joint_poses[0].translation);
	local_mat *= glm::mat4_cast(pose.joint_poses[0].rotation);
	local_mat = glm::scale(local_mat, pose.joint_poses[0].scale);
	std::vector<glm::mat4> global_joint_poses(pose.joint_poses.size());
	global_joint_poses[0] = local_mat;

	for (int i = 1; i < global_joint_poses.size(); i++)
	{
		auto& joint = skeleton.joints[i];
		auto& parent_global = global_joint_poses[joint.parent];

		glm::mat4 local_matty = glm::identity<glm::mat4>();
		local_matty = glm::translate(local_matty, pose.joint_poses[i].translation);
		local_matty *= glm::mat4_cast(pose.joint_poses[i].rotation);
		local_matty = glm::scale(local_matty, pose.joint_poses[i].scale);
		global_joint_poses[i] = parent_global * local_matty;
	}
	
	std::vector<glm::mat4> skinning_matrices(global_joint_poses.size());
	for (int i = 0; i < global_joint_poses.size(); i++)
	{
		skinning_matrices[i] = global_joint_poses[i] * ToMat4(skeleton.joints[i].local_to_joint);
	}

	shader.use();
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "skinning_matrices"), skinning_matrices.size(), GL_FALSE, glm::value_ptr(skinning_matrices[0]));

	glBindVertexArray(VAO);

	for (auto& mesh : meshes)
	{
		auto& material = materials[mesh.material_index];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, material.diffuse_map.id);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, material.specular_map.id);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, material.normal_map.id);
		shader.SetInt("material.diffuse", 0);
		shader.SetInt("material.specular", 1);
		shader.SetInt("material.normal", 2);
		shader.SetFloat("material.shininess", material.shininess);
		shader.SetVec3("material.diffuse_coeff", material.diffuse_coefficient);
		shader.SetVec3("material.specular_coeff", material.specular_coefficient);
		glDrawElements(GL_TRIANGLES, mesh.indices_end - mesh.indices_begin + 1, GL_UNSIGNED_INT, 0);
	}
}

void AnimatedModel::LoadAnimatedModel(const std::string& path)
{
	std::string directory = path.substr(0, path.find_last_of("/\\"));
	
	std::ifstream model_file_stream(path, std::ios::binary);

	ModelFile model_file_data;
	model_file_stream.read((char*)&model_file_data.header, sizeof(model_file_data.header));
	assert(model_file_data.header.magic_number == 'ldom');
	model_file_data.meshes = std::make_unique<Mesh[]>(model_file_data.header.num_meshes);
	model_file_stream.read((char*)model_file_data.meshes.get(), model_file_data.header.num_meshes * sizeof(Mesh));
	const auto has_tangents = HasFlag(model_file_data.header.vertex_flags, VertexFlags::HAS_TANGENT);
	const auto has_joint_data = HasFlag(model_file_data.header.vertex_flags, VertexFlags::HAS_JOINT_DATA);
	constexpr auto default_vertex_size_bytes = sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2); // position normal uv
	constexpr auto tangent_data_size_bytes = sizeof(glm::vec3);
	constexpr auto joint_data_size_bytes = sizeof(std::uint32_t) + sizeof(glm::vec4);
	const auto vertex_size_bytes = default_vertex_size_bytes + (has_tangents ? tangent_data_size_bytes : 0) + (has_joint_data ? joint_data_size_bytes : 0);
	const auto vertex_buffer_size_bytes = model_file_data.header.num_vertices * vertex_size_bytes;
	model_file_data.vertex_buffer = std::make_unique<std::uint8_t[]>(vertex_buffer_size_bytes);
	model_file_data.indices = std::make_unique<unsigned int[]>(model_file_data.header.num_indices);
	model_file_data.materials = std::make_unique<PhongMaterial[]>(model_file_data.header.num_materials);
	model_file_stream.read((char*)model_file_data.vertex_buffer.get(), vertex_buffer_size_bytes);
	const auto index_buffer_size_bytes = model_file_data.header.num_indices * sizeof(unsigned int);
	model_file_stream.read((char*)model_file_data.indices.get(), index_buffer_size_bytes);
	for (auto i = 0u; i < model_file_data.header.num_materials; i++)
	{
		auto& material = model_file_data.materials[i];
		model_file_stream.read((char*)&material.diffuse_coefficient, sizeof(material.diffuse_coefficient));
		model_file_stream.read((char*)&material.specular_coefficient, sizeof(material.specular_coefficient));
		model_file_stream.read((char*)&material.shininess, sizeof(material.shininess));
		std::string diffusemap_filename; std::getline(model_file_stream, diffusemap_filename, '\0');
		std::string specularmap_filename; std::getline(model_file_stream, specularmap_filename, '\0');
		std::string normalmap_filename; std::getline(model_file_stream, normalmap_filename, '\0');
		material.diffuse_map.id = LoadTexture(diffusemap_filename.c_str(), directory);
		material.specular_map.id = LoadTexture(specularmap_filename.c_str(), directory);
		material.normal_map.id = LoadTexture(normalmap_filename.c_str(), directory);
	}

	std::copy(model_file_data.meshes.get(), model_file_data.meshes.get() + model_file_data.header.num_meshes, std::back_inserter(this->meshes));
	std::copy(model_file_data.materials.get(), model_file_data.materials.get() + model_file_data.header.num_materials, std::back_inserter(this->materials));

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size_bytes, model_file_data.vertex_buffer.get(), GL_STATIC_DRAW);
	
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * model_file_data.header.num_indices, model_file_data.indices.get(), GL_STATIC_DRAW);

	char* offset = 0;
	std::uint32_t attribute_index = 0;

	// default vertex data- every vertex buffer has at least these things 
	// position
	glVertexAttribPointer(attribute_index, 3, GL_FLOAT, GL_FALSE, vertex_size_bytes, offset);
	glEnableVertexAttribArray(attribute_index);
	offset += sizeof(glm::vec3);
	attribute_index++;
	
	// normal
	glVertexAttribPointer(attribute_index, 3, GL_FLOAT, GL_FALSE, vertex_size_bytes, offset);
	glEnableVertexAttribArray(attribute_index);
	offset += sizeof(glm::vec3);
	attribute_index++;

	// uv
	glVertexAttribPointer(attribute_index, 2, GL_FLOAT, GL_FALSE, vertex_size_bytes, offset);
	glEnableVertexAttribArray(attribute_index);
	offset += sizeof(glm::vec2);
	attribute_index++;

	if (has_tangents)
	{
		// tangent
		glVertexAttribPointer(attribute_index, 3, GL_FLOAT, GL_FALSE, vertex_size_bytes, offset);
		glEnableVertexAttribArray(attribute_index);
		offset += sizeof(glm::vec3);
		attribute_index++;
	}
	if (has_joint_data)
	{
		// joint indices
		glVertexAttribIPointer(attribute_index, 1, GL_UNSIGNED_INT, vertex_size_bytes, offset);
		glEnableVertexAttribArray(attribute_index);
		offset += sizeof(std::uint32_t);
		attribute_index++;

		// joint weights
		glVertexAttribPointer(attribute_index, 4, GL_FLOAT, GL_FALSE, vertex_size_bytes, offset);
		glEnableVertexAttribArray(attribute_index);
		offset += sizeof(glm::vec4);
		attribute_index++;
	}

	std::ifstream skeleton_file_stream(directory + "/Warrok.skeleton", std::ios::binary);
	SkeletonFile skeleton_file_data;
	skeleton_file_stream.read((char*)&skeleton_file_data.header, sizeof(skeleton_file_data.header));
	assert(skeleton_file_data.header.magic_number == 'ntks');
	skeleton_file_data.joints = std::make_unique<Joint[]>(skeleton_file_data.header.num_joints);
	skeleton_file_data.joint_names = std::make_unique<std::string[]>(skeleton_file_data.header.num_joints);
	skeleton_file_stream.read((char*)skeleton_file_data.joints.get(), skeleton_file_data.header.num_joints * sizeof(Joint));
	for (auto i = 0u; i < skeleton_file_data.header.num_joints; i++)
	{
		auto& joint_name = skeleton_file_data.joint_names[i];
		std::getline(skeleton_file_stream, joint_name, '\0');
	}

	std::copy(skeleton_file_data.joints.get(), skeleton_file_data.joints.get() + skeleton_file_data.header.num_joints, std::back_inserter(this->skeleton.joints));
	std::copy(skeleton_file_data.joint_names.get(), skeleton_file_data.joint_names.get() + skeleton_file_data.header.num_joints, std::back_inserter(this->skeleton.joint_names));

	std::ifstream animation_file_stream(directory + "/sitting_laughing.animation", std::ios::binary);
	AnimationClipFile::Header clip_file_header;
	animation_file_stream.read((char*)&clip_file_header, sizeof(clip_file_header));
	clip.frames_per_second = clip_file_header.frames_per_second;
	clip.frame_count = clip_file_header.frame_count;
	clip.loops = clip_file_header.loops;
	const auto num_poses = clip_file_header.frame_count + (clip_file_header.loops ? 0 : 1);
	clip.poses.resize(num_poses);
	const auto pose_size_bytes = skeleton.joints.size() * sizeof(JointPose);
	for (auto& pose : clip.poses)
	{
		pose.joint_poses.resize(skeleton.joints.size());
		animation_file_stream.read((char*)pose.joint_poses.data(), pose_size_bytes);
		// TODO: fix this temporary fix
		for (auto& pose : pose.joint_poses) pose.rotation = glm::quat(pose.rotation.x, pose.rotation.y, pose.rotation.z, pose.rotation.w);
	}
}