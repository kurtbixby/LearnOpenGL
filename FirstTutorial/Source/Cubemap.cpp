#include "Headers/Cubemap.h"

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#include <boost/filesystem.hpp>

#include <vector>
#include <string>

#include <glad/glad.h>

#include "Headers/Texture.h"

const float Cubemap::Vertices_[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
};

Cubemap::Cubemap() : Cubemap(std::vector<std::string>
	{ "Resources/skybox/right.jpg", "Resources/skybox/left.jpg", "Resources/skybox/top.jpg", "Resources/skybox/bottom.jpg", "Resources/skybox/back.jpg", "Resources/skybox/front.jpg" }
) {}

Cubemap::Cubemap(uint32_t cubemapTextureName)
{
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    
    unsigned int vbo_;
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices_), &Vertices_[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, reinterpret_cast<void*>(0));
    glBindVertexArray(0);
    
    texId_ = cubemapTextureName;
}

Cubemap::Cubemap(std::vector<std::string> textureFiles) : Cubemap::Cubemap(load_cubemap(textureFiles))
{}

void Cubemap::Activate(Shader& shader, std::string cubemapUniformName)
{
    shader.SetInt(cubemapUniformName, SKYBOX_TEX_UNIT);
    glActiveTexture(GL_TEXTURE0 + SKYBOX_TEX_UNIT);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texId_);
}

void Cubemap::Deactivate()
{
    glActiveTexture(GL_TEXTURE0 + SKYBOX_TEX_UNIT);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Cubemap::Draw(Shader& shader)
{
	Activate(shader);
	glBindVertexArray(vao_);
	glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
	Deactivate();
}
