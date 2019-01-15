#version 330 core
out vec4 FragColor;

uniform vec3 OutColor;

void main()
{
    FragColor = vec4(OutColor, 1.0f);
}
