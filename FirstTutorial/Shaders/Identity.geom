#version 330 core
#define PRIM_VERTICES 3

layout (triangles) in;
layout (triangle_strip, max_vertices = PRIM_VERTICES) out;

in VS_OUT
{
    vec3 Normal;
    vec3 FragPos;
    vec2 TexCoords;
} gs_in[];

out VS_OUT
{
    vec3 Normal;
    vec3 FragPos;
    vec2 TexCoords;
} gs_out;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

void main()
{
    int i;
    for (i = 0; i < PRIM_VERTICES; i++)
    {
        gs_out.Normal = gs_in[i].Normal;
        gs_out.FragPos = gs_in[i].FragPos;
        gs_out.TexCoords = gs_in[i].TexCoords;
        gl_Position = projection * vec4(gs_in[i].FragPos, 1.0f);
        EmitVertex();
    }
    
    EndPrimitive();
}
