#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;

void main()
{
    vec3 depthValues = texture(depthMap, TexCoords).rgb;
    float depthValue = (depthValues.r + depthValues.g + depthValues.b) / 3.0f;
    FragColor = vec4(vec3(depthValue), 1.0f);
}
