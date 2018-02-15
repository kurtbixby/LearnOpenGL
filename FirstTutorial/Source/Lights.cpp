//
//  Lights.cpp
//  FirstTutorial
//
//  Created by Kurt Bixby on 2/4/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "Lights.h"
#include "Structs.h"

#define LIGHT_SIZE 60
#define POINT_LIGHT_SIZE 72
#define SPOT_LIGHT_SIZE 84

Light::Light(LightData data)
{
    data_ = data;
}

Light::Light(LightColorData color_data, glm::vec3 direction)
{
    data_.color_data = color_data;
    data_.direction = direction;
}

size_t Light::DataSize()
{
    // Take padding into account
    return LIGHT_SIZE;
}

size_t Light::BufferData(GLuint offset)
{
    uint32_t offset_offset = -offset % GLSL_STRUCT_ALIGNMENT;
    
    offset_offset += buffer_vec3(data_.direction, offset + offset_offset);
    offset_offset += buffer_LightColorData(data_.color_data, offset + offset_offset);
    
    return offset_offset;
}

uint32_t Light::GLSLSize()
{
    return LIGHT_SIZE;
}

glm::vec3 Light::Direction()
{
    return data_.direction;
}

void Light::ChangeDirection(glm::vec3 direction)
{
    data_.direction = direction;
}

PointLight::PointLight(PointLightData data)
{
    data_ = data;
}

PointLight::PointLight(LightColorData color_data, glm::vec3 position, float constant, float linear, float quadratic)
{
    data_.color_data = color_data;
    data_.position = position;
    data_.constant = constant;
    data_.linear = linear;
    data_.quadratic = quadratic;
}

size_t PointLight::DataSize()
{
    return POINT_LIGHT_SIZE;
}

size_t PointLight::BufferData(GLuint offset)
{
    uint32_t offset_offset = -offset % GLSL_STRUCT_ALIGNMENT;
    
    offset_offset += buffer_vec3(data_.position, offset + offset_offset);
    offset_offset += buffer_LightColorData(data_.color_data, offset + offset_offset);
    
    glBufferSubData(GL_UNIFORM_BUFFER, offset + offset_offset, sizeof(data_.constant), &data_.constant);
    offset_offset += sizeof(data_.constant);
    
    glBufferSubData(GL_UNIFORM_BUFFER, offset + offset_offset, sizeof(data_.linear), &data_.linear);
    offset_offset += sizeof(data_.linear);
    
    glBufferSubData(GL_UNIFORM_BUFFER, offset + offset_offset, sizeof(data_.quadratic), &data_.quadratic);
    offset_offset += sizeof(data_.quadratic);
    
    return offset_offset;
}

uint32_t PointLight::GLSLSize()
{
    return POINT_LIGHT_SIZE;
}

void PointLight::ChangePosition(glm::vec3 position)
{
    data_.position = position;
}

glm::vec3 PointLight::Position()
{
    return data_.position;
}

SpotLight::SpotLight(SpotLightData data)
{
    data_ = data;
}

SpotLight::SpotLight(LightColorData color_data, glm::vec3 position, glm::vec3 direction, float inner_cutoff, float outer_cutoff)
{
    data_.color_data = color_data;
    data_.position = position;
    data_.direction = direction;
    data_.innerCutoff = inner_cutoff;
    data_.outerCutoff = outer_cutoff;
}

size_t SpotLight::DataSize()
{
    return SPOT_LIGHT_SIZE;
}

size_t SpotLight::BufferData(GLuint offset)
{
    uint32_t offset_offset = -offset % GLSL_STRUCT_ALIGNMENT;
    
    offset_offset += buffer_vec3(data_.position, offset + offset_offset);
    offset_offset += buffer_vec3(data_.direction, offset + offset_offset);
    
    offset_offset += buffer_LightColorData(data_.color_data, offset + offset_offset);
    
    glBufferSubData(GL_UNIFORM_BUFFER, offset + offset_offset, sizeof(data_.innerCutoff), &data_.innerCutoff);
    offset_offset += sizeof(data_.innerCutoff);
    
    glBufferSubData(GL_UNIFORM_BUFFER, offset + offset_offset, sizeof(data_.outerCutoff), &data_.outerCutoff);
    offset_offset += sizeof(data_.outerCutoff);
    
    return offset_offset;
}

uint32_t SpotLight::GLSLSize()
{
    return SPOT_LIGHT_SIZE;
}

glm::vec3 SpotLight::Position()
{
    return data_.position;
}

void SpotLight::ChangePosition(glm::vec3 position)
{
    data_.position = position;
}

glm::vec3 SpotLight::Direction()
{
    return data_.direction;
}

void SpotLight::ChangeDirection(glm::vec3 direction)
{
    data_.direction = direction;
}
