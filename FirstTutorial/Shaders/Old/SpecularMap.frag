#version 330 core
struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct Light {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

uniform Material material;
uniform Light light;

uniform mat4 view;

void main()
{
	vec3 diffuseValue = vec3(texture(material.diffuse, TexCoords));
	vec3 specularValue = vec3(texture(material.specular, TexCoords));

	vec3 ambient = light.ambient * diffuseValue;

	vec3 viewLightPos = vec3(view * vec4(light.position, 1.0f));
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(viewLightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = light.diffuse * diff * diffuseValue;

	vec3 viewDir = normalize(-FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = light.specular * spec * specularValue;

	vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0f);
}