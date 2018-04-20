#ifndef STRUCTS_INLINE_H
#define STRUCTS_INLINE_H

#include <string>
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

inline uint32_t buffer_vec3(glm::vec3& vec, uint32_t offset)
{
    uint32_t misalignment = offset % VEC3_ALIGNMENT;
    uint32_t buffered = misalignment ? VEC3_ALIGNMENT - misalignment : misalignment;
    glBufferSubData(GL_UNIFORM_BUFFER, offset + buffered, sizeof(vec), &vec);
    buffered += sizeof(vec);
    return buffered;
}

#endif
