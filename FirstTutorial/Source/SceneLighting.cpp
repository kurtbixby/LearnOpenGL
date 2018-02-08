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

SceneLighting::SceneLighting(std::vector<Light> lights, std::vector<PointLight> pointLights, std::vector<SpotLight> spotLights)
{
    lights_ = lights;
    pointLights_ = pointLights;
    spotLights_ = spotLights;
}

size_t SceneLighting::DataSize()
{
    size_t size = 0;
    
    size += 3 * sizeof(GLuint);
    
//    if (lights_.size() > 0)
//    {
        size += lights_.size() * MAX_DIR_LIGHTS;
//    }
//    if (pointLights_.size() > 0)
//    {
        size += pointLights_.size() * MAX_POINT_LIGHTS;
//    }
//    if (spotLights_.size() > 0)
//    {
        size += spotLights_.size() * MAX_SPOT_LIGHTS;
//    }
    
    return size;
}

size_t SceneLighting::BufferData(GLuint offset)
{
    GLuint offset_offset = 0;
    
    // Buffer the number of each light type
    int lights_num = lights_.size();
    glBufferSubData(GL_UNIFORM_BUFFER, offset + offset_offset, sizeof(lights_num), &lights_num);
    offset_offset += sizeof(lights_num);
    
    int pt_light_num = pointLights_.size();
    glBufferSubData(GL_UNIFORM_BUFFER, offset + offset_offset, sizeof(pt_light_num), &pt_light_num);
    offset_offset += sizeof(pt_light_num);
    
    int spt_light_num = spotLights_.size();
    glBufferSubData(GL_UNIFORM_BUFFER, offset + offset_offset, sizeof(spt_light_num), &spt_light_num);
    offset_offset += sizeof(spt_light_num);

    for (int i = 0; i < lights_.size(); i++) {
        offset_offset += lights_[i].BufferData(offset + offset_offset);
    }
    offset_offset += sizeof(Light) * (MAX_DIR_LIGHTS - lights_num);
    
    for (int i = 0; i < pointLights_.size(); i++) {
        offset_offset += pointLights_[i].BufferData(offset + offset_offset);
    }
    offset_offset += sizeof(PointLight) * (MAX_POINT_LIGHTS - pt_light_num);
    
    for (int i = 0; i < spotLights_.size(); i++) {
        offset_offset += spotLights_[i].BufferData(offset + offset_offset);
    }
    offset_offset += sizeof(SpotLight) * (MAX_SPOT_LIGHTS - spt_light_num);
    
    return offset_offset;
}
