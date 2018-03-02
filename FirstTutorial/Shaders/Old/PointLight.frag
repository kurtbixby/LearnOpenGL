#version 330 core
struct Material {
	sampler2D diffuse;
	sampler2D specular;
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

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

uniform Material material;
uniform Light light;
uniform PointLight pointLight;

uniform mat4 view;

vec3 global_lighting(vec3 diffuseValue, vec3 specularValue)
{
		vec3 ambient = light.ambient * diffuseValue;

		vec3 viewLightDir = normalize(-vec3(view * vec4(light.direction, 1.0f)));
		vec3 norm = normalize(Normal);
		float diff = max(dot(norm, viewLightDir), 0.0f);
		vec3 diffuse = light.diffuse * diff * diffuseValue;

		vec3 viewDir = normalize(-FragPos);
		vec3 reflectDir = reflect(-viewLightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular = light.specular * spec * specularValue;
		vec3 result = ambient + diffuse + specular;

		return result;
}

vec3 point_lighting(vec3 diffuseValue, vec3 specularValue)
{
		vec3 ambient = pointLight.ambient * diffuseValue;

		vec3 viewLightPos = vec3(view * vec4(pointLight.position, 1.0f));
		vec3 norm = normalize(Normal);
		vec3 lightDir = normalize(viewLightPos - FragPos);
		float diff = max(dot(norm, lightDir), 0.0f);
		vec3 diffuse = pointLight.diffuse * diff * diffuseValue;

		vec3 viewDir = normalize(-FragPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular = pointLight.specular * spec * specularValue;

		float distance = length(viewLightPos - FragPos);
		float attenuation = 1.0f / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * (distance * distance));			

		vec3 result = attenuation * (ambient + diffuse + specular);

		return result;
}

void main()
{
	vec3 diffuseValue = vec3(texture(material.diffuse, TexCoords));
	vec3 specularValue = vec3(texture(material.specular, TexCoords));

	// Global Light
	vec3 global = global_lighting(diffuseValue, specularValue);

	// Point Light
	vec3 point = point_lighting(diffuseValue, specularValue);

	vec3 result = global + point;

	FragColor = vec4(result, 1.0f);
}