#version 330 core
#define IN_VERTICES 3
#define OUT_VERTICES 6

layout (triangles) in;
layout (line_strip, max_vertices = OUT_VERTICES) out;

in VS_OUT
{
    vec3 Normal;
    vec3 FragPos;
    vec2 TexCoords;
} gs_in[];

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

float magnitude = 1.0f;

void main()
{
    int i;
    for (i = 0; i < IN_VERTICES; i++)
    {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
        vec4 proj_normal = normalize(projection * vec4(gs_in[i].Normal, 0.0f));
        gl_Position = gl_in[i].gl_Position + proj_normal * magnitude;
        EmitVertex();
        EndPrimitive();
    }
}

