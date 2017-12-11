#ifndef MODEL_CPP
#define MODEL_CPP

#include "Model.h"

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
#include "Shader.h"
#include "Structs_Inline.h"

unordered_map<string, Texture> Model::loaded_textures_ = unordered_map<string, Texture>();

void Model::Draw(Shader shader)
{
	for (unsigned int i = 0; i < meshes_.size(); i++)
	{
		meshes_[i].Draw(shader);
	}
}

void Model::loadModel(boost::filesystem::path path)
{
	std::string path_string = path.string();
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path_string, aiProcess_Triangulate | aiProcess_FlipUVs);

	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    directory_ = path.parent_path().string();

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes_.push_back(processMesh(mesh, scene));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
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
	std::vector<Texture> textures;
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	    vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::Diffuse);
	    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	    vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::Specular);
	    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType texType)
{
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
        mat->GetTexture(type, i, &str);

		boost::filesystem::path texturePath = boost::filesystem::path(directory_).append(str.C_Str()).make_preferred();
		string pathString = texturePath.string();

		auto texture_iterator = loaded_textures_.find(pathString);
		if (texture_iterator == loaded_textures_.end())
		{
			// Not loaded
			std::cout << pathString << " not loaded" << std::endl;
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), directory_);
			texture.type = texType;
			texture.path = str;

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

unsigned int Model::TextureFromFile(const char* fileName, std::string directory)
{
	boost::filesystem::path texturePath = boost::filesystem::path(directory).append(fileName).make_preferred();
	return load_texture(texturePath.string().c_str());
}

#endif
