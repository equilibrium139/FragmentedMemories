#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <chrono>
#include <iostream>

#include "Texture.h"


void Model::draw(Shader& shader)
{
	for (int i = 0; i < meshes.size(); i++)
	{
		meshes[i].draw(shader);
	}
}

void Model::loadModel(std::string path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		// TODO: set model data == error model or something. Ex rainbow cube.
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << '\n';
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));
	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices(mesh->mNumVertices);
	for (std::size_t i = 0; i < vertices.size(); i++)
	{
		aiVector3D& vec = mesh->mVertices[i];
		vertices[i].position = glm::vec3(vec.x, vec.y, vec.z);
		if (mesh->HasNormals())
		{
			aiVector3D& norm = mesh->mNormals[i];
			vertices[i].normal = glm::vec3(norm.x, norm.y, norm.z);
		}
		if (mesh->mTextureCoords[0])
		{
			aiVector3D& texCo = mesh->mTextureCoords[0][i]; // We only use the first set of tex Coords (Assimp allows up to 8)
			vertices[i].texCoord = glm::vec2(texCo.x, texCo.y);
		}
		else {
			vertices[i].texCoord = glm::vec2(0.0f, 0.0f);
		}
	}

	std::vector<unsigned int> indices;
	for (std::size_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace& face = mesh->mFaces[i];
		for (std::size_t j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	std::vector<Texture> textures;
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::Diffuse);
		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::Specular);
		
		diffuseMaps.insert(diffuseMaps.end(), specularMaps.begin(), specularMaps.end());
		textures = std::move(diffuseMaps);
	}

	return Mesh( vertices, indices, textures, mesh->mName.C_Str() );
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType texType)
{
	unsigned int textureCount = mat->GetTextureCount(type);
	std::vector<Texture> textures(textureCount);
	for (unsigned int i = 0; i < textureCount; i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		Texture& texture = textures[i];
		texture.id = LoadTexture(str.C_Str(), directory);
		texture.type = texType;
	}
	return textures;
}
