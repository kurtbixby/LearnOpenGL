//
//  TextureCreator.cpp
//  FirstTutorial
//
//  Created by Kurt Bixby on 5/2/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#include "Headers/TextureCreator.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

GLuint TextureCreator::CreateTexture(TextureOptions options, uint32_t width, uint32_t height, uint32_t samples, void* dataPtr)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    
    bool multisampled = samples > 1;
    auto sampleType = multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    
    glBindTexture(sampleType, texture);
    
    if (multisampled)
    {
        glTexImage2DMultisample(sampleType, samples, options.TexFormat, width, height, GL_TRUE);
    }
    else
    {
        glTexImage2D(sampleType, 0, options.TexFormat, width, height, 0, options.SourceDataFormat, options.SourceDataType, dataPtr);
        glTexParameteri(sampleType, GL_TEXTURE_MIN_FILTER, options.MinFilter);
        glTexParameteri(sampleType, GL_TEXTURE_MAG_FILTER, options.MagFilter);
        glTexParameteri(sampleType, GL_TEXTURE_WRAP_S, options.WrapType);
        glTexParameteri(sampleType, GL_TEXTURE_WRAP_T, options.WrapType);
    }
    
    if (options.SourceDataFormat == GL_DEPTH_COMPONENT || options.SourceDataFormat == GL_DEPTH_STENCIL)
    {
        glTexParameterfv(sampleType, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4(1.0f)));
    }
    
    glBindTexture(sampleType, 0);
    return texture;
}

GLuint TextureCreator::CreateCubemapTexture(TextureOptions options, uint32_t width, uint32_t height, void* dataPtr)
{
    unsigned int cubemap;
    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    for (int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, options.TexFormat, width, height, 0, options.SourceDataFormat, options.SourceDataType, dataPtr);
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, options.MinFilter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, options.MagFilter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, options.WrapType);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, options.WrapType);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, options.WrapType);
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return cubemap;
}
