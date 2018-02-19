//
//  SceneLighting.cpp
//  FirstTutorial
//
//  Created by Kurt Bixby on 2/4/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "Headers/Lights.h"
#include "Headers/SceneLighting.h"
#include "Headers/Structs.h"

#define DIR_LIGHT_OFFSET 16
#define PNT_LIGHT_OFFSET 80
#define SPT_LIGHT_OFFSET 400

SceneLighting::SceneLighting(std::vector<Light> lights, std::vector<PointLight> pointLights, std::vector<SpotLight> spotLights)
{
    lights_ = lights;
    pointLights_ = pointLights;
    spotLights_ = spotLights;
}

size_t SceneLighting::DataSize()
{
    size_t size = 0;
    
    size += 3 * sizeof(uint32_t);
    
    size += (size % GLSL_STRUCT_ALIGNMENT) ? GLSL_STRUCT_ALIGNMENT - (size % GLSL_STRUCT_ALIGNMENT) : (size % GLSL_STRUCT_ALIGNMENT);
    uint32_t light_size = Light::GLSLSize();
    uint32_t light_padding = (light_size % GLSL_STRUCT_ALIGNMENT) ? GLSL_STRUCT_ALIGNMENT - (light_size % GLSL_STRUCT_ALIGNMENT) : (light_size % GLSL_STRUCT_ALIGNMENT);
    size += ((MAX_DIR_LIGHTS - 1) * (light_size + light_padding)) + light_size;
    
    size += (size % GLSL_STRUCT_ALIGNMENT) ? GLSL_STRUCT_ALIGNMENT - (size % GLSL_STRUCT_ALIGNMENT) : (size % GLSL_STRUCT_ALIGNMENT);
    uint32_t pt_light_size = PointLight::GLSLSize();
    uint32_t pt_light_padding = (pt_light_size % GLSL_STRUCT_ALIGNMENT) ? GLSL_STRUCT_ALIGNMENT - (pt_light_size % GLSL_STRUCT_ALIGNMENT) : (pt_light_size % GLSL_STRUCT_ALIGNMENT);
    size += ((MAX_POINT_LIGHTS - 1) * (pt_light_size + pt_light_padding)) + pt_light_size;
    
    size += (size % GLSL_STRUCT_ALIGNMENT) ? GLSL_STRUCT_ALIGNMENT - (size % GLSL_STRUCT_ALIGNMENT) : (size % GLSL_STRUCT_ALIGNMENT);
    uint32_t spot_light_size = SpotLight::GLSLSize();
    uint32_t spot_light_padding = (spot_light_size % GLSL_STRUCT_ALIGNMENT) ? GLSL_STRUCT_ALIGNMENT - (spot_light_size % GLSL_STRUCT_ALIGNMENT) : (spot_light_size % GLSL_STRUCT_ALIGNMENT);
    size += ((MAX_SPOT_LIGHTS - 1) * (spot_light_size + spot_light_padding)) + spot_light_size;
    
    return size;
}

size_t SceneLighting::BufferData(GLuint offset)
{
    GLuint offset_offset = 0;
    
    // Buffer the number of each light type
    uint32_t lights_num = lights_.size();
    glBufferSubData(GL_UNIFORM_BUFFER, offset + offset_offset, sizeof(lights_num), &lights_num);
    offset_offset += sizeof(lights_num);
    
    uint32_t pt_light_num = pointLights_.size();
    glBufferSubData(GL_UNIFORM_BUFFER, offset + offset_offset, sizeof(pt_light_num), &pt_light_num);
    offset_offset += sizeof(pt_light_num);
    
    uint32_t spt_light_num = spotLights_.size();
    glBufferSubData(GL_UNIFORM_BUFFER, offset + offset_offset, sizeof(spt_light_num), &spt_light_num);
    offset_offset += sizeof(spt_light_num);

    offset_offset = DIR_LIGHT_OFFSET;
    for (int i = 0; i < lights_.size(); i++) {
        offset_offset += (offset_offset % GLSL_STRUCT_ALIGNMENT) ? GLSL_STRUCT_ALIGNMENT - (offset_offset % GLSL_STRUCT_ALIGNMENT) : (offset_offset % GLSL_STRUCT_ALIGNMENT);
        offset_offset += lights_[i].BufferData(offset + offset_offset);
    }
    
    offset_offset = PNT_LIGHT_OFFSET;
    for (int i = 0; i < pointLights_.size(); i++) {
        offset_offset += (offset_offset % GLSL_STRUCT_ALIGNMENT) ? GLSL_STRUCT_ALIGNMENT - (offset_offset % GLSL_STRUCT_ALIGNMENT) : (offset_offset % GLSL_STRUCT_ALIGNMENT);
        offset_offset += pointLights_[i].BufferData(offset + offset_offset);
    }
    
    offset_offset = SPT_LIGHT_OFFSET;
    for (int i = 0; i < spotLights_.size(); i++) {
        offset_offset += (offset_offset % GLSL_STRUCT_ALIGNMENT) ? GLSL_STRUCT_ALIGNMENT - (offset_offset % GLSL_STRUCT_ALIGNMENT) : (offset_offset % GLSL_STRUCT_ALIGNMENT);
        offset_offset += spotLights_[i].BufferData(offset + offset_offset);
    }
    
    return DataSize();
}
