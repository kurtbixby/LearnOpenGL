#ifndef MODEL_H
#define MODEL_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <iostream>
#include <vector>

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#include <boost/filesystem.hpp>

#include "Mesh.h"
#include "Shader.h"

class Model
{
	public:
		Model(std::string path)
		{
			loadModel(path);
		}
		void Draw(Shader shader);
	private:
		std::vector<Mesh> meshes_;
		std::string directory_;

		void loadModel(std::string path);
        void processNode(aiNode* node, const aiScene* scene);
        Mesh processMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType texType);

        static unsigned int TextureFromFile(const char* fileName, std::string directory);
};

void Model::Draw(Shader shader)
{
	for (unsigned int i = 0; i < meshes_.size(); i++)
	{
		meshes_[i].Draw(shader);
	}
}

void Model::loadModel(std::string path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    directory_ = path.substr(0, path.find_last_of('/'));

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
	    std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::Diffuse);
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

        // Check for already loaded texture

        Texture texture;
        texture.id = TextureFromFile(str.C_Str(), directory_);
        texture.type = texType;
        texture.path = str;
        textures.push_back(texture);
	}

	return textures;
}

unsigned int Model::TextureFromFile(const char* fileName, std::string directory)
{
	boost::filesystem::path texturePath = boost::filesystem::path(directory).append(fileName).make_preferred();
	return load_texture(texturePath.string().c_str());
}

#endif
