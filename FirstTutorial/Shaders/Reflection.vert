#version 330 core
layout (location = 0) vec3 aPos;
layout (location = 1) vec3 aNormal;

out vec3 Position;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	Normal = vec3(mat3(transpose(inverse(model))) * aNormal);
	Position = vec3(model * vec4(aPos, 1.0f));
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}
