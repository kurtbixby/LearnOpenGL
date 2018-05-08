//
//  TextureCreator.h
//  FirstTutorial
//
//  Created by Kurt Bixby on 5/2/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#ifndef TEXTURECREATOR_H
#define TEXTURECREATOR_H

#include <stdio.h>

#include <glad/glad.h>

struct TextureOptions
{
    GLuint TexFormat = GL_RGB;
    GLuint SourceDataFormat = GL_RGB;
    GLuint SourceDataType = GL_UNSIGNED_BYTE;
    GLuint WrapType = GL_REPEAT;
    GLuint MagFilter = GL_NEAREST;
    GLuint MinFilter = GL_NEAREST;
};

class TextureCreator {
    
public:
    static unsigned int CreateTexture(TextureOptions options, uint32_t width, uint32_t height, uint32_t samples = 1, void* dataPtr = nullptr);
    static unsigned int CreateCubemapTexture(TextureOptions options, uint32_t width, uint32_t height, void* dataPtr = nullptr);
};

#endif /* TEXTURECREATOR_H */
