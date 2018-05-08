#ifndef STRUCTS_H
#define STRUCTS_H

#include <glm/glm.hpp>

#define VEC3_ALIGNMENT 16
#define GLSL_STRUCT_ALIGNMENT 16

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

enum class FBAttachment
{
    Color,
    Depth,
    DepthHiRes,
    DepthStencil,
    ColorHDR
};

enum class RenderTargetType
{
    None,
    Texture,
    Renderbuffer,
    Cubemap
};

struct RenderTarget
{
    unsigned int TargetName;
    RenderTargetType TargetType;
};

Vertex create_vertex(float posX, float posY, float posZ, float normX, float normY, float normZ, float texX, float texY);
Vertex create_vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texCoords);
uint32_t buffer_vec3(glm::vec3& vec3, uint32_t offset);

#include "Structs_Inline.h"

#endif
