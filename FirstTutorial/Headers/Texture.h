//
//  Texture.h
//  FirstTutorial
//
//  Created by Kurt Bixby on 4/20/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <glad/glad.h>

#define MAX_DIFFUSE_TEX 3
#define MAX_SPECULAR_TEX 3

#define SKYBOX_TEX_UNIT 15
#define DIR_SHAD_MAP_TEX_START 20

enum class TextureType
{
    Diffuse,
    Specular,
    Reflection,
    Normal
};

struct Texture
{
    unsigned int id;
    TextureType type;
    std::string path;
};

Texture load_texture(std::string directory, std::string filename, TextureType texType);
unsigned int texture_from_file(const char* fileName, std::string directory);
void load_texture_file(const char* texture_file, const GLenum texture_type = GL_TEXTURE_2D, const bool linear_space = true);
unsigned int load_texture(std::string texture_path, const bool linear_space = true, const GLenum wrap_type = GL_REPEAT);
unsigned int load_cubemap(std::vector<std::string> texturePaths);

#include "Texture_Inline.h"

#endif /* TEXTURE_H */
