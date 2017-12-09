#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <assimp/types.h>
#include <boost/filesystem.hpp>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#define MAX_DIFFUSE_TEX 3
#define MAX_SPECULAR_TEX 3

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

enum class TextureType
{
	Diffuse,
	Specular
};

struct Texture
{
	unsigned int id;
	TextureType type;
	aiString path;
};

class Object
{
	public:
		glm::vec3 Transform_;
		int Mesh_;
		glm::vec3 Scale_;
		bool Outlined_;

		Object() : Object(glm::vec3(1.0f), -1, 1.0f, false) {};
		Object(const glm::vec3 transform, const int mesh, const float scale, const bool outline)
		{
			Transform_ = transform;
			Mesh_ = mesh;
			Scale_ = glm::vec3(scale);
			Outlined_ = outline;
		}
};

Vertex create_vertex(float posX, float posY, float posZ, float normX, float normY, float normZ, float texX, float texY);
Texture load_texture(std::string directory, std::string filename, TextureType texType);
unsigned int texture_from_file(const char* fileName, std::string directory);
unsigned int load_texture(const char* texture_file, const GLenum source_format = GL_RGBA, const GLenum wrap_type = GL_REPEAT);

// Keep this separate to keep Vertex as POD
inline Vertex create_vertex(float posX, float posY, float posZ, float normX, float normY, float normZ, float texX, float texY)
{
    Vertex v;
    v.Position = glm::vec3(posX, posY, posZ);
    v.Normal = glm::vec3(normX, normY, normZ);
    v.TexCoords = glm::vec2(texX, texY);

    return v;
}

inline Vertex create_vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texCoords)
{
	Vertex v;
	v.Position = position;
	v.Normal = normal;
	v.TexCoords = texCoords;

	return v;
}

inline Texture load_texture(std::string directory, std::string filename, TextureType texType)
{
	Texture texture;
	texture.id = texture_from_file(filename.c_str(), directory);
	texture.type = texType;
	texture.path = filename;

	return texture;
}

inline unsigned int texture_from_file(const char* fileName, std::string directory)
{
	boost::filesystem::path texturePath = boost::filesystem::path(directory).append(fileName).make_preferred();
	return load_texture(texturePath.string().c_str());
}

inline unsigned int load_texture(const char* texture_file, const GLenum source_format, const GLenum wrap_type)
{
	std::cout << "LOADING: " << texture_file << std::endl;
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_type);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_type);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width, height, nr_channels;
	unsigned char* data = stbi_load(texture_file, &width, &height, &nr_channels, 0);

	if (data)
	{

		/*
		* 1) OpenGL object type
		* 2) Mipmap level
		* 3) Format to store texture in
		* 4) Texture width
		* 5) Texture height
		* 6) Legacy stuff [what exactly?]
		* 7) Format of source texture
		* 8) Datatype of source texture
		* 9) Actual texture data
		*/
		glTexImage2D(GL_TEXTURE_2D, 0, source_format, width, height, 0, source_format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);

	return texture;
}

#endif
