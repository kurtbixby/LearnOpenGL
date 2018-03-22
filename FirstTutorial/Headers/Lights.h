#ifndef LIGHTS_H
#define LIGHTS_H

#include <glm/glm.hpp>

#include "IBufferable.h"
#include "Structs.h"

#define MAX_DIR_LIGHTS 1
#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 1

#define DIR_SHAD_MAP_TEX_START 20
#define PNT_SHAD_MAP_TEX_START DIR_SHAD_MAP_TEX_START + MAX_DIR_LIGHTS
#define SPT_SHAD_MAP_TEX_START PNT_SHAD_MAP_TEX_START + MAX_POINT_LIGHTS

struct LightColorData
{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

inline uint32_t buffer_LightColorData(LightColorData& color_data, uint32_t offset)
{
    uint32_t offset_offset = 0;
    
    offset_offset += buffer_vec3(color_data.ambient, offset + offset_offset);
    offset_offset += buffer_vec3(color_data.diffuse, offset + offset_offset);
    offset_offset += buffer_vec3(color_data.specular, offset + offset_offset);
    
    return offset_offset;
}

struct LightData
{
    glm::vec3 direction;
    
    LightColorData color_data;
};

struct PointLightData
{
    glm::vec3 position;
    
    LightColorData color_data;
    
    float constant;
    float linear;
    float quadratic;
};

struct SpotLightData
{
    glm::vec3 direction;
    glm::vec3 position;
    
    LightColorData color_data;
    
    float innerCutoff;
    float outerCutoff;
};

class Light : public IBufferable {
public:
    Light(LightData data);
    Light(LightColorData color_data, glm::vec3 direction);
    
    size_t DataSize();
    size_t BufferData(GLuint offset);
    
    static uint32_t GLSLSize();
    
    void ChangeDirection(glm::vec3 direction);
    glm::vec3 Direction();
    
    LightData data_;
};

// Make PointLight contain a Light?
class PointLight : public IBufferable {
public:
    PointLight(PointLightData data);
    PointLight(LightColorData color_data, glm::vec3 position);
    PointLight(LightColorData color_data, glm::vec3 position, float constant, float linear, float quadratic);
    
    size_t DataSize();
    size_t BufferData(GLuint offset);
    
    static uint32_t GLSLSize();
    
    void ChangePosition(glm::vec3 position);
    glm::vec3 Position();
    
    PointLightData data_;
};

class SpotLight : public IBufferable {
public:
    SpotLight(SpotLightData data);
    SpotLight(LightColorData color_data, glm::vec3 position, glm::vec3 direction, float inner_cutoff, float outer_cutoff);
    
    size_t DataSize();
    size_t BufferData(GLuint offset);
    
    static uint32_t GLSLSize();
    
    void ChangePosition(glm::vec3 position);
    void ChangeDirection(glm::vec3 direction);
    
    glm::vec3 Position();
    glm::vec3 Direction();
    
    float LightAngle();
    
    SpotLightData data_;
};

struct ColorConstants
{
	inline static glm::vec3 RedLight()
	{
		return glm::vec3(1.0f, 0.0f, 0.0f);
	}

	inline static glm::vec3 GreenLight()
	{
		return glm::vec3(0.0f, 1.0f, 0.0f);
	}

	inline static glm::vec3 BlueLight()
	{
		return glm::vec3(0.0f, 0.0f, 1.0f);
	}

	inline static glm::vec3 WhiteLight()
	{
		return glm::vec3(1.0f, 1.0f, 1.0f);
	}
};

#endif
