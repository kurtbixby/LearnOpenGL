#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <assimp/types.h>

#include "Shader.h"

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

enum class FBAttachment
{
	Color,
	// Depth,
	// Stencil,
	DepthStencil
};

enum class RenderTargetType
{
	None,
	Texture,
	Renderbuffer
};

struct RenderTarget
{
	unsigned int TargetName;
	RenderTargetType TargetType;
};

Vertex create_vertex(float posX, float posY, float posZ, float normX, float normY, float normZ, float texX, float texY);
Vertex create_vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texCoords);
Texture load_texture(std::string directory, std::string filename, TextureType texType);
unsigned int texture_from_file(const char* fileName, std::string directory);
unsigned int load_texture(const char* texture_file, const GLenum source_format = GL_RGBA, const GLenum wrap_type = GL_REPEAT);

#include "Structs_Inline.h"

#endif
