#version 330 core
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform mat4 view;
uniform samplerCube skybox;

void main()
{
	vec3 incident = normalize(FragPos);
	vec3 norm = normalize(Normal);
	vec3 reflected = reflect(incident, norm);
	// this feels ugly and requires excess processing?
	reflected = vec3(inverse(view) * vec4(reflected, 0.0f));
	FragColor = vec4(texture(skybox, reflected).rgb, 1.0f);
}
