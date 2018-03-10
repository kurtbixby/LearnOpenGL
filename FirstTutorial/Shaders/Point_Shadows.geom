#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

out vec4 FragPos;

uniform mat4 lightSpace[6];

void main()
{
    for (int i = 0; i < 6; i++)
    {
        gl_Layer = i;
        for (int i = 0; i < 3; i++)
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = lightSpace[i] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}
