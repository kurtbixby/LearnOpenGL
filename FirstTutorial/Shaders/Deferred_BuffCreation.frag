#version 330 core

#define MAX_DIFFUSE_TEXS 1
#define MAX_SPECULAR_TEXS 1
#define MAX_REFLECTION_MAPS 1

layout (location = 0) out vec3 outWorldPosition;
layout (location = 1) out vec3 outWorldNormal;
layout (location = 2) out vec3 outColorDiffuse;
// Change specular to be a coef for diffuse color
layout (location = 3) out vec3 outColorSpec;
layout (location = 4) out vec3 outReflectDir;
// Change reflect map to be coef for reflect color
layout (location = 5) out vec3 outReflectColor;

struct Material {
    sampler2D diffuse[MAX_DIFFUSE_TEXS];
    sampler2D specular[MAX_SPECULAR_TEXS];
    sampler2D reflection[MAX_REFLECTION_MAPS];
    float shininess;
};

in VS_OUT
{
    vec2 TexCoords;
    vec3 FragWorldPos;
    vec3 WorldNormal;
} fs_in;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

uniform vec3 cam_position;

uniform Material material;

uniform int DIFFUSE_TEXS;
uniform int SPECULAR_TEXS;
uniform int REFLECTION_MAPS;

vec3 reflection_dir()
{
    vec3 incident = normalize(fs_in.FragWorldPos - cam_position);
    vec3 reflected = normalize(reflect(incident, fs_in.WorldNormal));
    
    return reflected;
}

void main()
{
    vec3 diffuseValue = vec3(0.0f);
    if (DIFFUSE_TEXS > 0)
    {
        diffuseValue = vec3(texture(material.diffuse[0], fs_in.TexCoords));
    }
    
    vec3 specularValue = vec3(0.0f);
    if (SPECULAR_TEXS > 0)
    {
        specularValue = texture(material.specular[0], fs_in.TexCoords);
    }
    
    vec3 reflectionDir = vec3(0.0f);
    vec3 reflectionColor = vec3(0.0f);
    if (REFLECTION_MAPS > 0)
    {
        reflectionDir = reflection_dir();
        reflectionColor = texture(material.reflection[0], fs_in.TexCoords);
    }
    
    outWorldPosition = fs_in.FragWorldPos;
    outWorldNormal = fs_in.WorldNormal;
    outColorDiffuse = diffuseValue;
    outColorSpec = specularValue;
    outReflectDir = reflectionDir;
    outReflectColor = reflectionColor;
}
