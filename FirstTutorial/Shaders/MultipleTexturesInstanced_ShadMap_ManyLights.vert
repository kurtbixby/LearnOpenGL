#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceMatrix;

out VS_OUT
{
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoords;
    vec4 FragWorldPos;
} vs_out;

layout (std140) uniform Matrices
{
	mat4 projection;
	mat4 view;
};

void main()
{
    gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0f);
	vs_out.Normal = mat3(transpose(inverse(view * instanceMatrix))) * aNorm;
	vs_out.FragPos = vec3(view * instanceMatrix * vec4(aPos, 1.0f));
	vs_out.TexCoords = aTexCoords;
    vs_out.FragWorldPos = instanceMatrix * vec4(aPos, 1.0f);
}
