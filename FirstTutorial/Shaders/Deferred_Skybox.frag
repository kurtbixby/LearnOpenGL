#version 330 core
layout (location = 6) out vec4 outSkyboxColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    outSkyboxColor = vec4(texture(skybox, TexCoords).rgb, 1.0f);
}
