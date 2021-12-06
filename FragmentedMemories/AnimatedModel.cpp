#include "AnimatedModel.h"

#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cassert>
#include <iostream>
#include <utility>
#include "stb_image.h"

static void PushChildrenOf(std::vector<aiNode*>& nodes, std::vector<int>& parent, unsigned int begin);
static std::pair<std::vector<aiNode*>, std::vector<int>> FlattenNodeHierarchy(aiNode* root);
static glm::mat4x3 ToGLMMat(const aiMatrix4x4& mat);

// All asserts here document some of my assumptions

AnimatedModel::AnimatedModel(const std::string& path)
{
	LoadAnimatedModel(path);
}

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

void AnimatedModel::Draw(Shader& shader, float dt)
{
	static float time = 0.0f;
	time += dt;
	float clip_duration = clip.frame_count / clip.frames_per_second;
	float pose_index = std::fmod(time, clip_duration) * clip.frames_per_second;
	auto& pose = clip.poses[pose_index];

	glm::mat4 local_mat = glm::identity<glm::mat4>();
	local_mat = glm::translate(local_mat, pose.local_joint_poses[0].translation);
	local_mat *= glm::mat4_cast(pose.local_joint_poses[0].rotation);
	local_mat = glm::scale(local_mat, pose.local_joint_poses[0].scale);
	pose.global_joint_poses[0] = local_mat;

	for (int i = 1; i < pose.global_joint_poses.size(); i++)
	{
		auto& joint = skeleton.joints[i];
		auto& parent_global = pose.global_joint_poses[joint.parent];

		glm::mat4 local_matty = glm::identity<glm::mat4>();
		local_matty = glm::translate(local_matty, pose.local_joint_poses[i].translation);
		local_matty *= glm::mat4_cast(pose.local_joint_poses[i].rotation);
		local_matty = glm::scale(local_matty, pose.local_joint_poses[i].scale);
		pose.global_joint_poses[i] = parent_global * local_matty;
	}
	
	std::vector<glm::mat4> skinning_matrices(pose.global_joint_poses.size());
	for (int i = 0; i < pose.global_joint_poses.size(); i++)
	{
		skinning_matrices[i] = pose.global_joint_poses[i] * ToMat4(skeleton.joints[i].local_to_joint);
	}

	shader.use();
	glUniformMatrix4fv(glGetUniformLocation(shader.id, "skinning_matrices"), skeleton.joints.size(), GL_FALSE, glm::value_ptr(skinning_matrices[0]));

	glBindVertexArray(VAO);

	for (auto& mesh : meshes)
	{
		mesh.Draw(shader);
	}
}

void AnimatedModel::LoadAnimatedModel(const std::string& path)
{
	assert(AI_LMW_MAX_WEIGHTS == 4);
	Assimp::Importer importer;
	unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_LimitBoneWeights | aiProcess_ValidateDataStructure | aiProcess_GenSmoothNormals |
						 aiProcess_CalcTangentSpace;
	const aiScene* scene = importer.ReadFile(path, flags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << '\n';
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));

	auto flattened_nodes = FlattenNodeHierarchy(scene->mRootNode);
	auto& nodes = flattened_nodes.first;
	auto& parent = flattened_nodes.second;

	CreateSkeleton(scene, nodes, parent);
	CreateMeshes(scene);
	CreateAnimationClip(scene);
}

void AnimatedModel::CreateSkeleton(const aiScene* scene, const std::vector<aiNode*>& nodes, const std::vector<int>& parent)
{
	const auto num_nodes = nodes.size();

	// Mark all nodes which correspond to a bone
	std::vector<aiBone*> node_bones(num_nodes, nullptr);
	const auto num_meshes = scene->mNumMeshes;
	for (auto i = 0u; i < num_meshes; i++)
	{
		auto mesh = scene->mMeshes[i];

		const auto num_bones = mesh->mNumBones;
		for (auto i = 0u; i < num_bones; i++)
		{
			auto bone = mesh->mBones[i];

			for (auto k = 0u; k < num_nodes; k++)
			{
				if (bone->mName == nodes[k]->mName)
				{
					node_bones[k] = bone;
					break;
				}
			}
		}
	}

	// Add only nodes which correspond to a bone to the skeleton
	for (auto i = 0u; i < num_nodes; i++)
	{
		auto bone = node_bones[i];

		if (bone)
		{
			skeleton.joints.emplace_back();
			Joint& joint = skeleton.joints.back();
			skeleton.joint_names.emplace_back(bone->mName.C_Str());
			joint.local_to_joint = ToGLMMat(bone->mOffsetMatrix);

			auto node_parent_index = parent[i];
			if (node_parent_index >= 0 && node_bones[node_parent_index] != nullptr)
			{
				auto parent_iter = std::find(skeleton.joint_names.begin(), skeleton.joint_names.end(), nodes[node_parent_index]->mName.C_Str());
				assert(parent_iter != skeleton.joint_names.end());
				joint.parent = std::distance(skeleton.joint_names.begin(), parent_iter);
			}
			else joint.parent = -1;
		}
	}
}

void AnimatedModel::CreateMeshes(const aiScene* scene)
{
	for (auto mesh_index = 0u; mesh_index < scene->mNumMeshes; mesh_index++)
	{
		auto mesh = scene->mMeshes[mesh_index];

		meshes.emplace_back();
		SkinnedMesh& skinned_mesh = meshes.back();
		skinned_mesh.indices_begin = indices.size();
		
		// Append mesh indices to model's index list, offsetting them by the appropriate amount
		const unsigned int mesh_vertices_begin = vertices.size();
		const auto num_faces = mesh->mNumFaces;
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			auto face = mesh->mFaces[i];
			indices.insert(indices.end(), { face.mIndices[0] + mesh_vertices_begin, face.mIndices[1] + mesh_vertices_begin, face.mIndices[2] + mesh_vertices_begin });
		}
		skinned_mesh.index_count = indices.size() - skinned_mesh.indices_begin;

		assert(mesh->HasBones() && mesh->HasNormals() && mesh->HasTextureCoords(0));

		// Append mesh vertices to model's vertex list
		const auto num_vertices = mesh->mNumVertices;
		vertices.resize(vertices.size() + num_vertices);
		for (unsigned int i = 0; i < num_vertices; i++)
		{
			auto vert = mesh->mVertices[i];
			auto normal = mesh->mNormals[i];
			auto uv = mesh->mTextureCoords[0][i];
			auto tangent = mesh->mTangents[i];
			auto bitangent = mesh->mBitangents[i];
			const auto vertex_index = i + mesh_vertices_begin;
			vertices[vertex_index].position = { vert.x, vert.y, vert.z };
			vertices[vertex_index].normal = { normal.x, normal.y, normal.z };
			vertices[vertex_index].tex_coords = { uv.x, uv.y };
			vertices[vertex_index].joint_indices = { 0, 0, 0, 0 };
			vertices[vertex_index].joint_weights = { 0.0f, 0.0f, 0.0f, 0.0f };
			vertices[vertex_index].tangent = { tangent.x, tangent.y, tangent.z };
			vertices[vertex_index].bitangent = { bitangent.x, bitangent.y, bitangent.z };
		}

		// Each mesh stores the bones that affect its vertices. Loop through these bones and 
		// determine which vertices are affected by which bones.
		std::vector<int> num_joints_affecting(num_vertices, 0);
		const auto num_bones = mesh->mNumBones;
		const auto num_joints = skeleton.joints.size();
		for (int i = 0; i < num_bones; i++)
		{
			auto bone = mesh->mBones[i];

			auto joint_corresponding_to_bone = 0;
			while (joint_corresponding_to_bone < num_joints && bone->mName.C_Str() != skeleton.joint_names[joint_corresponding_to_bone]) joint_corresponding_to_bone++;
			assert(joint_corresponding_to_bone < num_joints);

			auto vertices_affected = bone->mNumWeights;
			for (int j = 0; j < vertices_affected; j++)
			{
				auto weight = bone->mWeights[j];
				SkinnedVertex& vertex = vertices[weight.mVertexId + mesh_vertices_begin];
				auto& joints_affecting_vertex = num_joints_affecting[weight.mVertexId];
				vertex.joint_indices[joints_affecting_vertex] = joint_corresponding_to_bone;
				vertex.joint_weights[joints_affecting_vertex] = weight.mWeight;
				joints_affecting_vertex++;
			}
		}

		// Load textures
		auto mesh_material = scene->mMaterials[mesh->mMaterialIndex];

		aiColor3D diffuse_color;
		mesh_material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color);

		aiColor3D specular_color;
		mesh_material->Get(AI_MATKEY_COLOR_SPECULAR, specular_color);

		skinned_mesh.material.diffuse_coefficient = { diffuse_color.r, diffuse_color.g, diffuse_color.b };
		skinned_mesh.material.specular_coefficient = { specular_color.r, specular_color.g, specular_color.b };

		mesh_material->Get(AI_MATKEY_SHININESS, skinned_mesh.material.shininess);

		// This assumes at most one diffuse and at most one specular maps
		aiString texture_path;
		if (mesh_material->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), texture_path) != aiReturn_FAILURE)
		{
			if (auto texture = scene->GetEmbeddedTexture(texture_path.C_Str()))
			{
				if (texture->mHeight == 0)
				{
					std::string identifier = texture->mFilename.C_Str();
					auto last_slash_index = identifier.find_last_of("/");
					assert(last_slash_index != std::string::npos);
					identifier = identifier.substr(last_slash_index + 1);
					skinned_mesh.material.diffuse_map.id = LoadTexture((unsigned char*)texture->pcData, texture->mWidth, identifier);
				}
			}
			else
			{
				// TODO: Load texture from file on disk
			}
		}
		else
		{
			std::cout << mesh->mName.C_Str() << " has no diffuse map\n";
		}

		if (mesh_material->Get(AI_MATKEY_TEXTURE_SPECULAR(0), texture_path) != aiReturn_FAILURE)
		{
			if (auto texture = scene->GetEmbeddedTexture(texture_path.C_Str()))
			{
				if (texture->mHeight == 0)
				{
					std::string identifier = texture->mFilename.C_Str();
					auto last_slash_index = identifier.find_last_of("/");
					assert(last_slash_index != std::string::npos);
					identifier = identifier.substr(last_slash_index + 1);
					skinned_mesh.material.specular_map.id = LoadTexture((unsigned char*)texture->pcData, texture->mWidth, identifier);
				}
			}
			else
			{
				// TODO: Load texture from file on disk
			}
		}
		else
		{
			std::cout << mesh->mName.C_Str() << " has no specular map\n";
		}

		if (mesh_material->Get(AI_MATKEY_TEXTURE_NORMALS(0), texture_path) != aiReturn_FAILURE)
		{
			if (auto texture = scene->GetEmbeddedTexture(texture_path.C_Str()))
			{
				if (texture->mHeight == 0)
				{
					std::string identifier = texture->mFilename.C_Str();
					auto last_slash_index = identifier.find_last_of("/");
					assert(last_slash_index != std::string::npos);
					identifier = identifier.substr(last_slash_index + 1);
					skinned_mesh.material.normal_map.id = LoadTexture((unsigned char*)texture->pcData, texture->mWidth, identifier);
				}
			}
			else
			{
				// TODO: Load texture from file on disk
			}
		}
		else
		{
			std::cout << mesh->mName.C_Str() << " has no specular map\n";
		}
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(SkinnedVertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, tex_coords));

	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, joint_indices));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, joint_weights));

	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, tangent));

	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, bitangent));

	glBindVertexArray(0);
}

static JointPose ToJointPose(aiMatrix4x4& mat)
{
	aiQuaternion quat;
	aiVector3D trans;
	aiVector3D scale;
	mat.Decompose(scale, quat, trans);
	JointPose joint_pose;
	joint_pose.rotation = glm::quat(quat.w, quat.x, quat.y, quat.z);
	joint_pose.translation = { trans.x, trans.y, trans.z };
	joint_pose.scale = { scale.x, scale.y, scale.z };
	return joint_pose;
}

void AnimatedModel::CreateAnimationClip(const aiScene* scene)
{
	assert(scene->mNumAnimations == 1);

	auto animation = scene->mAnimations[0];

	assert(animation->mTicksPerSecond != 0.0);

	bind_pose.resize(skeleton.joints.size());
	auto pair = FlattenNodeHierarchy(scene->mRootNode);
	auto flattened_nodes = pair.first;
	for (int i = 0; i < bind_pose.size(); i++)
	{
		auto& joint_name = skeleton.joint_names[i];
		for (int j = 0; j < flattened_nodes.size(); j++)
		{
			if (joint_name == flattened_nodes[j]->mName.C_Str())
			{
				bind_pose[i] = ToJointPose(flattened_nodes[j]->mTransformation);
			}
		}
	}

	clip.frames_per_second = animation->mTicksPerSecond;
	clip.frame_count = animation->mDuration;
	clip.poses.resize(clip.frame_count);
	const auto identity = glm::identity<glm::mat4>();
	const JointPose identity_pose = { glm::identity<glm::quat>(), glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{1.0f, 1.0f, 1.0f} };

	for (auto& skeleton_pose : clip.poses)
	{
		skeleton_pose.local_joint_poses = bind_pose;
		skeleton_pose.global_joint_poses.resize(skeleton.joints.size(), glm::zero<glm::mat4>());
	}

	for (int i = 0; i < animation->mNumChannels; i++)
	{
		auto node_animation = animation->mChannels[i];
		assert(node_animation->mNumPositionKeys == node_animation->mNumRotationKeys && node_animation->mNumPositionKeys == node_animation->mNumScalingKeys);

		auto node_joint_index = 0u;
		while (node_joint_index < skeleton.joint_names.size() && skeleton.joint_names[node_joint_index] != node_animation->mNodeName.C_Str()) node_joint_index++;
		assert(node_joint_index < skeleton.joint_names.size());

		// For now nodes with only one key frame will simply be duplicated across all key frames
		if (node_animation->mNumPositionKeys == 1)
		{
			auto quat = node_animation->mRotationKeys[0].mValue;
			auto trans = node_animation->mPositionKeys[0].mValue;
			auto scale = node_animation->mScalingKeys[0].mValue;
			JointPose joint_pose;
			joint_pose.rotation = glm::quat(quat.w, quat.x, quat.y, quat.z);
			joint_pose.translation = { trans.x, trans.y, trans.z };
			joint_pose.scale = { scale.x, scale.y, scale.z };
			for (auto& skeleton_pose : clip.poses)
			{
				skeleton_pose.local_joint_poses[node_joint_index] = joint_pose;
			}
		}
		else
		{
			for (int pose = 0; pose < clip.frame_count; pose++)
			{
				auto quat = node_animation->mRotationKeys[pose].mValue;
				auto trans = node_animation->mPositionKeys[pose].mValue;
				auto scale = node_animation->mScalingKeys[pose].mValue;

				if (node_animation->mRotationKeys[pose].mTime != node_animation->mPositionKeys[pose].mTime || node_animation->mRotationKeys[pose].mTime != node_animation->mScalingKeys[pose].mTime)
				{
					std::cout << "Well shit.. I done diddly fucked up\n";
				}

				JointPose joint_pose;
				joint_pose.rotation = glm::quat(quat.w, quat.x, quat.y, quat.z);
				joint_pose.translation = { trans.x, trans.y, trans.z };
				joint_pose.scale = { scale.x, scale.y, scale.z };
				clip.poses[pose].local_joint_poses[node_joint_index] = joint_pose;
			}
		}
	}
}

void PushChildrenOf(std::vector<aiNode*>& nodes, std::vector<int>& parent, unsigned int begin)
{
	unsigned int end = nodes.size();
	for (unsigned int i = begin; i < end; i++)
	{
		auto node = nodes[i];
		for (unsigned int j = 0; j < node->mNumChildren; j++)
		{
			nodes.push_back(node->mChildren[j]);
			parent.push_back(i);
		}
	}
	if (end < nodes.size())
	{
		PushChildrenOf(nodes, parent, end);
	}
}

std::pair<std::vector<aiNode*>, std::vector<int>> FlattenNodeHierarchy(aiNode* root)
{
	std::vector<aiNode*> flattened;
	std::vector<int> parent;
	flattened.push_back(root);
	parent.push_back(-1);
	PushChildrenOf(flattened, parent, 0);
	return { flattened, parent };
}

static glm::mat4x3 ToGLMMat(const aiMatrix4x4& mat)
{
	glm::mat4x3 glm_mat;

	glm_mat[0][0] = mat.a1;
	glm_mat[0][1] = mat.b1;
	glm_mat[0][2] = mat.c1;

	glm_mat[1][0] = mat.a2;
	glm_mat[1][1] = mat.b2;
	glm_mat[1][2] = mat.c2;

	glm_mat[2][0] = mat.a3;
	glm_mat[2][1] = mat.b3;
	glm_mat[2][2] = mat.c3;

	glm_mat[3][0] = mat.a4;
	glm_mat[3][1] = mat.b4;
	glm_mat[3][2] = mat.c4;

	return glm_mat;
}

void SkinnedMesh::Draw(Shader& shader)
{
	shader.use();

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
	glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, (void*)indices_begin);
}
