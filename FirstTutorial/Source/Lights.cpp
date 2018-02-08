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
    return sizeof(data_);
}

size_t Light::BufferData(GLuint offset)
{
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(data_), &data_);
    
    return sizeof(data_);
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
    return sizeof(data_);
}

size_t PointLight::BufferData(GLuint offset)
{
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(data_), &data_);
    
    return sizeof(data_);
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
    return sizeof(data_);
}

size_t SpotLight::BufferData(GLuint offset)
{
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(data_), &data_);
    
    return sizeof(data_);
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
