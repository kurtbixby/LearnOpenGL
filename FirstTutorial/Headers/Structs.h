#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

#define MAX_DIFFUSE_TEX 3
#define MAX_SPECULAR_TEX 3
#define VEC3_ALIGNMENT 16
#define GLSL_STRUCT_ALIGNMENT 16

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

enum class TextureType
{
	Diffuse,
	Specular,
	Reflection
};

struct Texture
{
	unsigned int id;
	TextureType type;
	std::string path;
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

struct CameraInput
{
	bool MoveForward : 1;
    bool MoveBack : 1;
	bool MoveLeft : 1;
	bool MoveRight : 1;
	bool RotateRight : 1;
	bool RotateLeft : 1;
	char : 2;
	float x_delta;
	float y_delta;
	float y_offset;
};

struct Input
{
	CameraInput cameraInput;
};

Vertex create_vertex(float posX, float posY, float posZ, float normX, float normY, float normZ, float texX, float texY);
Vertex create_vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texCoords);
Texture load_texture(std::string directory, std::string filename, TextureType texType);
unsigned int texture_from_file(const char* fileName, std::string directory);
void load_texture_file(const char* texture_file, const GLenum texture_type = GL_TEXTURE_2D);
unsigned int load_texture(std::string texture_path, const GLenum wrap_type = GL_REPEAT);
unsigned int load_cubemap(std::vector<std::string> texturePaths);
uint32_t buffer_vec3(glm::vec3& vec3, uint32_t offset);

#include "Structs_Inline.h"

#endif
