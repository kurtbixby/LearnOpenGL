//
//  ModelTexture_Inline.h
//  FirstTutorial
//
//  Created by Kurt Bixby on 4/20/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#ifndef MODEL_TEXTURE_INLINE_h
#define MODEL_TEXTURE_INLINE_h

#include <string>
#include <boost/filesystem.hpp>

inline ModelTexture load_texture(std::string directory, std::string filename, ModelTextureType texType)
{
    ModelTexture texture;
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

#endif /* MODEL_TEXTURE_INLINE_h */
