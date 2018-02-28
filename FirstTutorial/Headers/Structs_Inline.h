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
	return load_texture(texturePath.string());
}

inline unsigned int load_texture(std::string texture_path, const bool linear_space, const GLenum wrap_type)
{
	const char* texture_file = texture_path.c_str();
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_type);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_type);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	load_texture_file(texture_file, GL_TEXTURE_2D, linear_space);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}

inline unsigned int load_cubemap(std::vector<std::string> texturePaths)
{
	unsigned int cubemapTexId;
	glGenTextures(1, &cubemapTexId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexId);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for (int i = 0; i < 6; i++)
	{
		const char* texture_file = texturePaths[i].c_str();
		load_texture_file(texture_file, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, false);
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return cubemapTexId;
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
