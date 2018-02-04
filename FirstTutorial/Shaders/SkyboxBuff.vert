#version 330 core
layout (location = 0) in vec3 aPos;

layout (std140) uniform Matrices
{
	mat4 projection;
	mat4 view;
};

out vec3 TexCoords;

void main()
{
	TexCoords = aPos;
	gl_Position = projection * mat4(mat3(view)) * vec4(aPos, 1.0);
}
