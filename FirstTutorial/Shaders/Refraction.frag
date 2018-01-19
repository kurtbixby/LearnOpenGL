#version 330 core
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 camPosition;
uniform mat4 view;
uniform samplerCube skybox;

void main()
{
	float outsideMat = 1.00f;
	float insideMat = 1.52f;
	float refractionRatio = outsideMat / insideMat;
	vec3 incident = normalize(FragPos);
	vec3 norm = normalize(Normal);
	vec3 refracted = refract(incident, norm, refractionRatio);
	// this feels ugly and requires excess processing?
	refracted = vec3(inverse(view) * vec4(refracted, 0.0f));
	FragColor = vec4(texture(skybox, refracted).rgb, 1.0f);
}
