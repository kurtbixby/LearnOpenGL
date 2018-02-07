//
//  SceneLighting.cpp
//  FirstTutorial
//
//  Created by Kurt Bixby on 2/4/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "Lights.h"
#include "SceneLighting.h"

size_t SceneLighting::DataSize()
{
    size_t size = 0;
    
    if (lights_.size() > 0)
    {
        size += lights_.size() * lights_[0].DataSize();
    }
    if (pointLights_.size() > 0)
    {
        size += pointLights_.size() * pointLights_[0].DataSize();
    }
    if (spotLights_.size() > 0)
    {
        size += spotLights_.size() * spotLights_[0].DataSize();
    }
    
    return size;
}

size_t SceneLighting::BufferData(GLuint offset)
{
    GLuint offset_offset = 0;
    for (int i = 0; i < lights_.size(); i++) {
        offset_offset += lights_[i].BufferData(offset_offset);
    }
    
    for (int i = 0; i < pointLights_.size(); i++) {
        offset_offset += pointLights_[i].BufferData(offset_offset);
    }
    
    for (int i = 0; i < spotLights_.size(); i++) {
        offset_offset += spotLights_[i].BufferData(offset_offset);
    }
    
    return offset_offset;
}
