#version 330 core

#define MAX_LIGHTS 2

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTan;
layout (location = 4) in vec3 aBiTan;

struct Light {      // 60 bytes needs 4 bytes of padding
    vec3 position; //  0
    
    vec3 ambient;   // 16
    vec3 diffuse;   // 32
    vec3 specular;  // 48
};

out VS_OUT
{
    vec2 TexCoords;
    vec3 TBNViewPos;
    vec3 TBNFragPos;
} vs_out;

out TBN_Lighting
{
    Light lights[MAX_LIGHTS];
} tbn_lighting;

layout (std140) uniform Lighting
{
    Light lights[MAX_LIGHTS];
};

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

uniform vec3 viewPos;
uniform mat4 model;

void main()
{
    vec3 Tan = normalize(vec3(model * vec4(aTan, 0.0f)));
    vec3 Norm = normalize(vec3(model * vec4(aNorm, 0.0f)));
//    vec3 BiTan = normalize(vec3(model * vec4(aBiTan, 0.0f)));
    vec3 BiTan = cross(Norm, Tan);
    mat3 InvTBN = transpose(mat3(Tan, BiTan, Norm));
    
    vs_out.TexCoords = aTexCoords;
    vs_out.TBNFragPos = InvTBN * aPos;
    vs_out.TBNViewPos = InvTBN * viewPos;
    
    int i;
    for (i = 0; i < MAX_LIGHTS; i++)
    {
        tbn_lighting.lights[i].position = InvTBN * lights[i].position;
        tbn_lighting.lights[i].ambient = lights[i].ambient;
        tbn_lighting.lights[i].diffuse = lights[i].diffuse;
        tbn_lighting.lights[i].specular = lights[i].specular;
    }
    
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
}
