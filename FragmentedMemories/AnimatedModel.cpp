#include "AnimatedModel.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cassert>
#include <iostream>

AnimatedModel::AnimatedModel(const std::string& path)
{
	LoadAnimatedModel(path);
}

void AnimatedModel::LoadAnimatedModel(const std::string& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << '\n';
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));
	ProcessNode(scene->mRootNode, scene);
}

void AnimatedModel::ProcessNode(const aiNode* node, const aiScene* scene)
{
	for (int i = 0; i < node->mNumMeshes; i++)
	{
		auto mesh_index = node->mMeshes[i];
		meshes.push_back(ProcessMeshAndBones(scene->mMeshes[mesh_index]));
	}
}

SkinnedMesh AnimatedModel::ProcessMeshAndBones(const aiMesh* mesh)
{
	SkinnedMesh skinned_mesh;

	const auto num_faces = mesh->mNumFaces;
	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		auto face = mesh->mFaces[i];
		assert(face.mNumIndices == 3);
		skinned_mesh.indices.insert(skinned_mesh.indices.end(), { face.mIndices[0], face.mIndices[1], face.mIndices[2] });
	}

	const auto num_vertices = mesh->mNumVertices;
	skinned_mesh.vertices.resize(num_vertices);
	for (int i = 0; i < num_vertices; i++)
	{
		auto vert = mesh->mVertices[i];
		auto normal = mesh->mNormals[i];
		auto uv = mesh->mTextureCoords[0][i];
		skinned_mesh.vertices[i].position = { vert.x, vert.y, vert.z };
		skinned_mesh.vertices[i].normal = { normal.x, normal.y, normal.z };
		skinned_mesh.vertices[i].tex_coords = { uv.x, uv.y };
		skinned_mesh.vertices[i].joint_indices = { 0, 0, 0, 0 };
		skinned_mesh.vertices[i].joint_weights = { 0.0f, 0.0f, 0.0f };
	}

	const auto num_bones = mesh->mNumBones;
	for (int i = 0; i < num_bones; i++)
	{
		auto bone = mesh->mBones[i];
		
	}
}
