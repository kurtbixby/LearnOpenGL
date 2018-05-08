//
//  ModelTexture.cpp
//  FirstTutorial
//
//  Created by Kurt Bixby on 4/20/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#include "Headers/ModelTexture.h"

#include <glad/glad.h>
#include <iostream>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "Headers/stb_image.h"
#endif

void load_texture_file(const char* texture_file, const GLenum texture_type, const bool linear_space)
{
    std::cerr << "LOADING: " << texture_file << std::endl;
    int width, height, nr_channels;
    unsigned char* data = stbi_load(texture_file, &width, &height, &nr_channels, 0);
    
    GLenum source_format;
    GLenum storage_format;
    switch(nr_channels)
    {
        case 1:
        source_format = GL_RED;
        break;
        case 3:
        source_format = linear_space ? GL_RGB : GL_SRGB;
        storage_format = GL_RGB;
        break;
        default:
        source_format = linear_space ? GL_RGBA : GL_SRGB_ALPHA;
        storage_format = GL_RGBA;
        break;
    }
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
        glTexImage2D(texture_type, 0, source_format, width, height, 0, storage_format, GL_UNSIGNED_BYTE, data);
        if (texture_type == GL_TEXTURE_2D)
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }
    else
    {
        std::cerr << "Failed to load texture" << std::endl;
    }
    
    stbi_image_free(data);
}
