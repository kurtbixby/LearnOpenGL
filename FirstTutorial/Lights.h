#ifndef LIGHTS_H
#define LIGHTS_H

#include <glm/glm.hpp>

#define MAX_DIR_LIGHTS 1
#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 1

struct Light {
	glm::vec3 direction;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

// Make PointLight contain a Light?
struct PointLight {
	glm::vec3 position;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

struct SpotLight {
	glm::vec3 position;
	glm::vec3 direction;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	float innerCutoff;
	float outerCutoff;
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
