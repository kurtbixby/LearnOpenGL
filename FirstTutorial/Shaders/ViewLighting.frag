#version 330 core
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform mat4 view;

void main()
{
	vec3 viewLightPos = vec3(view * vec4(lightPos, 1.0f));

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(viewLightPos - FragPos);

	float diffStrength = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diffStrength * lightColor;

	float ambientStrength = 0.1f;
	vec3 ambient = ambientStrength * lightColor;

	float specStrength = 0.5f;
	vec3 viewDir = normalize(-FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32) * specStrength;
	vec3 specular = spec * lightColor;

	vec3 result = (ambient + diffuse + specular) * objectColor;
	FragColor = vec4(result, 1.0);
}