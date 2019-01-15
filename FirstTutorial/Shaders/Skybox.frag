#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
	FragColor = vec4(texture(skybox, TexCoords).rgb, 1.0f);
    BrightColor = vec4(vec3(0.0f), 1.0f);
}
