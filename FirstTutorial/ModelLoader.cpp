#include "ModelLoader.h"

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#include <boost/filesystem.hpp>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Structs_Inline.h"

unordered_map<string, Texture> ModelLoader::loaded_textures_ = unordered_map<string, Texture>();

ModelLoader::ModelLoader(boost::filesystem::path path)
{
	directory_ = path.make_preferred().string();
}

Model ModelLoader::loadModel(std::string file)
{
	std::string path_string = boost::filesystem::path(directory_).append(file).make_preferred().string();
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path_string, aiProcess_Triangulate | aiProcess_FlipUVs);

	std::vector<Mesh> meshes = std::vector<Mesh>();
	std::vector<std::vector<Texture>> mesh_textures = std::vector<std::vector<Texture>>();

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
	}
	else
	{
		processNode(scene->mRootNode, scene, meshes, mesh_textures);
	}

	return Model(meshes, mesh_textures);
}

void ModelLoader::processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes, std::vector<std::vector<Texture>>& mesh_textures)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		std::vector<Texture> new_mesh_textures = std::vector<Texture>();
		meshes.push_back(processMesh(mesh, scene, new_mesh_textures));
		mesh_textures.push_back(new_mesh_textures);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, meshes, mesh_textures);
	}
}

Mesh ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene, std::vector<Texture>& textures)
{
	// Vertices
	std::vector<Vertex> vertices;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		glm::vec3 position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		glm::vec3 normals = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		glm::vec2 texCoords;
		if (mesh->mTextureCoords[0])
		{
			texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		else
		{
			texCoords = glm::vec2(0.0f, 0.0f);
		}

		vertices.push_back(create_vertex(position, normals, texCoords));
	}

	// Indices
	// All faces are now triangles
	// Will have 3 * faces
	// Face is 3 consecutives indices
	std::vector<unsigned int> indices;
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// Textures
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::Diffuse);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::Specular);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		vector<Texture> reflectionMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, TextureType::Reflection);
		textures.insert(textures.end(), reflectionMaps.begin(), reflectionMaps.end());
	}

	return Mesh(vertices, indices);
}

std::vector<Texture> ModelLoader::loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType texType)
{
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		boost::filesystem::path texturePath = boost::filesystem::path(directory_).append(str.C_Str()).make_preferred();
		//boost::filesystem::path texturePath = boost::filesystem::path(str.C_Str()).make_preferred();
		string pathString = texturePath.string();

		auto texture_iterator = loaded_textures_.find(pathString);
		if (texture_iterator == loaded_textures_.end())
		{
			// Not loaded
			std::cout << pathString << " not loaded" << std::endl;
			Texture texture;
			texture.id = load_texture(pathString.c_str());
			texture.type = texType;
			texture.path = str.C_Str();

			loaded_textures_.emplace(pathString, texture);

			textures.push_back(texture);
		}
		else
		{
			std::cout << pathString << " already loaded" << std::endl;
			textures.push_back(texture_iterator->second);
		}
	}

	return textures;
}
