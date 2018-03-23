#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture0;
uniform sampler2D texture1;

void main()
{
    vec3 color0 = texture(texture0, TexCoords).rgb;
    vec3 color1 = texture(texture1, TexCoords).rgb;
    
    FragColor = vec4(color0 + color1, 1.0f);
}
