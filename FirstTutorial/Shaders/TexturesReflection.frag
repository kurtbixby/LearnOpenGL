#version 330 core

#define MAX_DIFFUSE_TEXS 1
#define MAX_SPECULAR_TEXS 1
#define MAX_REFLECTION_MAPS 1

#define MAX_DIR_LIGHTS 1
#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 1

struct Material {
	sampler2D diffuse[MAX_DIFFUSE_TEXS];
	sampler2D specular[MAX_SPECULAR_TEXS];
	sampler2D reflection[MAX_REFLECTION_MAPS];
	float shininess;
};

struct Light {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

// Make PointLight contain a Light?
struct PointLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

struct SpotLight {
	vec3 position;
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float innerCutoff;
	float outerCutoff;
};

out vec4 FragColor;

in VS_OUT
{
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoords;
} fs_in;

layout (std140) uniform Matrices
{
	mat4 projection;
	mat4 view;
};

layout (std140) uniform Lighting
{
    int DIR_LIGHTS;
    int POINT_LIGHTS;
    int SPOT_LIGHTS;
    Light lights[MAX_DIR_LIGHTS];
    PointLight pointLights[MAX_POINT_LIGHTS];
    SpotLight spotLights[MAX_SPOT_LIGHTS];
};

uniform Material material;

uniform int DIFFUSE_TEXS;
uniform int SPECULAR_TEXS;
uniform int REFLECTION_MAPS;

uniform samplerCube skybox;

vec3 global_lighting(Light light, vec3 diffuseValue, vec3 specularValue)
{
	vec3 ambient = light.ambient * diffuseValue;

	vec3 viewLightDir = normalize(-vec3(view * vec4(light.direction, 1.0f)));
	vec3 norm = normalize(fs_in.Normal);
	float diff = max(dot(norm, viewLightDir), 0.0f);
	vec3 diffuse = light.diffuse * diff * diffuseValue;

	vec3 viewDir = normalize(-fs_in.FragPos);
	vec3 reflectDir = reflect(-viewLightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = light.specular * spec * specularValue;
	vec3 result = ambient + diffuse + specular;

	return result;
}

vec3 point_lighting(PointLight pointLight, vec3 diffuseValue, vec3 specularValue)
{
	vec3 ambient = pointLight.ambient * diffuseValue;

	vec3 viewLightPos = vec3(view * vec4(pointLight.position, 1.0f));
	vec3 norm = normalize(fs_in.Normal);
	vec3 lightDir = normalize(viewLightPos - fs_in.FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = pointLight.diffuse * diff * diffuseValue;

	vec3 viewDir = normalize(-fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = pointLight.specular * spec * specularValue;

	float distance = length(viewLightPos - fs_in.FragPos);
	float attenuation = 1.0f / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * (distance * distance));			

	vec3 result = attenuation * (ambient + diffuse + specular);

	return result;
}

vec3 spot_lighting(SpotLight spotLight, vec3 diffuseValue, vec3 specularValue)
{
	vec3 result = vec3(0.0f);

	vec3 viewLightPos = vec3(view * vec4(spotLight.position, 1.0f));
	vec3 lightDir = normalize(viewLightPos - fs_in.FragPos);

	float foo = dot(-lightDir, normalize(vec3(view * vec4(spotLight.direction, 0.0f))));
	if (foo > spotLight.outerCutoff)
	{
		float intensity = 1.0f;
		if (foo < spotLight.innerCutoff)
		{
			intensity = (foo - spotLight.outerCutoff) / (spotLight.innerCutoff - spotLight.outerCutoff);
		}

		vec3 ambient = spotLight.ambient * diffuseValue;

		vec3 norm = normalize(fs_in.Normal);
		float diff = max(dot(norm, lightDir), 0.0f);
		vec3 diffuse = spotLight.diffuse * diff * diffuseValue;

		vec3 viewDir = normalize(-fs_in.FragPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
		vec3 specular = spotLight.specular * spec * specularValue;

		result = intensity * (ambient + diffuse + specular);
	}
	return result;
}

vec3 skybox_reflection()
{	
	vec3 incident = normalize(fs_in.FragPos);
	vec3 norm = normalize(fs_in.Normal);
	vec3 reflected = reflect(incident, norm);
	// this feels ugly and requires excess processing?
	reflected = vec3(inverse(view) * vec4(reflected, 0.0f));

	vec3 reflectionColor = texture(skybox, reflected).rgb;
	vec3 reflectionIntensity = texture(material.reflection[0], fs_in.TexCoords).rgb;
	return reflectionColor * reflectionIntensity;
} 

void main()
{
	vec3 diffuseValue = vec3(0.0f);
	if (DIFFUSE_TEXS > 0)
	{
		diffuseValue = vec3(texture(material.diffuse[0], fs_in.TexCoords));
	}

	vec3 specularValue = vec3(0.0f);
	if (SPECULAR_TEXS > 0)
	{
		specularValue = vec3(texture(material.specular[0], fs_in.TexCoords));
	}
	
	vec3 reflection = vec3(0.0f);
	if (REFLECTION_MAPS > 0)
	{
		reflection = skybox_reflection();
	}

	int i = 0;

	// Global Light
	vec3 global = vec3(0.0f);
	for (i = 0; i < DIR_LIGHTS; i++)
	{
		global += global_lighting(lights[i], diffuseValue, specularValue);
	}

	// Point Light
	vec3 point = vec3(0.0f);
	for (i = 0; i < POINT_LIGHTS; i++)
	{
		point += point_lighting(pointLights[i], diffuseValue, specularValue);
	}

	// Spot Light
	vec3 spot = vec3(0.0f);
	for (i = 0; i < SPOT_LIGHTS; i++)
	{
		spot += spot_lighting(spotLights[i], diffuseValue, specularValue);
	}

	vec3 result = global + point + spot + reflection;

	FragColor = vec4(result, 1.0f);
}
