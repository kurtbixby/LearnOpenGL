#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float mixture;

void main()
{
	FragColor = mix(texture(texture0, TexCoord), texture(texture1, vec2(TexCoord.x, 1.0 - TexCoord.y)), mixture);
}