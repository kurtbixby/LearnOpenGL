#ifndef STRUCTS_INLINE_H
#define STRUCTS_INLINE_H

#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <boost/filesystem.hpp>

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

#endif
