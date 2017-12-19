#include "Structs.h"

#include <glad/glad.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

unsigned int load_texture(const char* texture_file, const GLenum source_format, const GLenum wrap_type)
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
