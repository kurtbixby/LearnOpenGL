#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT
{
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoords;
} vs_out;

layout (std140) uniform Matrices
{
	mat4 projection;
	mat4 view;
};

uniform mat4 model;

void main()
{
	vs_out.Normal = mat3(transpose(inverse(view * model))) * aNorm;
	vs_out.FragPos = vec3(view * model * vec4(aPos, 1.0f));
	vs_out.TexCoords = aTexCoords;
}
