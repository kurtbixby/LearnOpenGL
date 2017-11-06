#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct Texture
{
	unsigned int id;
	std::string type;
};

// Keep this separate to keep Vertex as POD
inline Vertex create_vertex(float posX, float posY, float posZ, float normX, float normY, float normZ, float texX, float texY)
{
    Vertex v;
    v.Position = glm::vec3(posX, posY, posZ);
    v.Normal = glm::vec3(normX, normY, normZ);
    v.TexCoords = glm::vec2(texX, texY);

    return v;
}

#endif
