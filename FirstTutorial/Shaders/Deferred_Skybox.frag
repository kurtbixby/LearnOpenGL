#version 330 core
layout (location = 0) out vec4 SkyboxColor;
layout (location = 1) out vec4 BrightColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    SkyboxColor = vec4(texture(skybox, TexCoords).rgb, 1.0f);
    BrightColor = vec4(vec3(0.0f), 1.0f);
}
