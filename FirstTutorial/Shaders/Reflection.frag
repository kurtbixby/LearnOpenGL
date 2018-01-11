#version 330 core
in vec3 Normal;
in vec3 Position;

out vec4 FragColor;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{
	vec3 incident = normalize(Position - cameraPos);
	vec3 reflected = reflect(incident, normalize(Normal));
	FragColor = vec4(texture(skybox, reflected).rgb, 1.0f);
}
