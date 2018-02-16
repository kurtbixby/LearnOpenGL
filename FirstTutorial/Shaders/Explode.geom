#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

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
//uniform float time;

vec3 GetNormal(vec3 norm0, vec3 norm1, vec3 norm2)
{
    vec3 v1 = norm1 - norm0;
    vec3 v2 = norm2 - norm0;
    
    return normalize(cross(v2, v1));
}

vec4 Explode(vec4 position, vec3 normal)
{
    float magnitude = 2.0f;
    vec3 direction = normal * magnitude * 1.0f;
    return position + vec4(direction, 0.0f);
}

void main()
{
//    vec3 proj_Normal = GetNormal(vec3(gl_in[0].gl_Position), vec3(gl_in[1].gl_Position), vec3(gl_in[2].gl_Position));
    vec3 view_Normal = GetNormal(gs_in[0].FragPos, gs_in[1].FragPos, gs_in[2].FragPos);
    
    vec4 view_Pos = Explode(vec4(gs_in[0].FragPos, 1.0f), view_Normal);
    gs_out.Normal = view_Normal;
    gs_out.FragPos = vec3(view_Pos);
    gs_out.TexCoords = gs_in[0].TexCoords;
    gl_Position = projection * view_Pos;
    EmitVertex();
    
    view_Pos = Explode(vec4(gs_in[1].FragPos, 1.0f), view_Normal);
    gs_out.Normal = view_Normal;
    gs_out.FragPos = vec3(view_Pos);
    gs_out.TexCoords = gs_in[1].TexCoords;
    gl_Position = projection * view_Pos;
    EmitVertex();
    
    view_Pos = Explode(vec4(gs_in[2].FragPos, 1.0f), view_Normal);
    gs_out.Normal = view_Normal;
    gs_out.FragPos = vec3(view_Pos);
    gs_out.TexCoords = gs_in[2].TexCoords;
    gl_Position = projection * view_Pos;
    EmitVertex();
    EndPrimitive();
}
