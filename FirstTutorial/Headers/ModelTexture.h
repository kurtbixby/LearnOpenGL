//
//  ModelTexture.h
//  FirstTutorial
//
//  Created by Kurt Bixby on 4/20/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#ifndef MODEL_TEXTURE_H
#define MODEL_TEXTURE_H

#include <string>
#include <glad/glad.h>

#define MAX_DIFFUSE_TEX 3
#define MAX_SPECULAR_TEX 3

#define DEFERRED_RENDER_TEX_UNIT 1
#define SKYBOX_TEX_UNIT 15
#define DIR_SHAD_MAP_TEX_START 20

enum class ModelTextureType
{
    DiffuseMap,
    SpecularMap,
    ReflectionMap,
    NormalMap
};

struct ModelTexture
{
    unsigned int id;
    ModelTextureType type;
    std::string path;
};

ModelTexture load_texture(std::string directory, std::string filename, ModelTextureType texType);
unsigned int texture_from_file(const char* fileName, std::string directory);
void load_texture_file(const char* texture_file, const GLenum texture_type = GL_TEXTURE_2D, const bool linear_space = true);
unsigned int load_texture(std::string texture_path, const bool linear_space = true, const GLenum wrap_type = GL_REPEAT);
unsigned int load_cubemap(std::vector<std::string> texturePaths);

#include "ModelTexture_Inline.h"

#endif /* MODEL_TEXTURE_H */
