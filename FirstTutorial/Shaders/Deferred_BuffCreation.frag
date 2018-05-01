#version 330 core

#define MAX_DIFFUSE_TEXS 1
#define MAX_SPECULAR_TEXS 1
#define MAX_REFLECTION_MAPS 1

layout (location = 0) out vec4 outWorldPosition;
layout (location = 1) out vec4 outWorldNormal;
layout (location = 2) out vec4 outColorDiffuse;
// Change specular to be a coef for diffuse color
layout (location = 3) out vec4 outColorSpec;
layout (location = 4) out vec4 outReflectDir;
// Change reflect map to be coef for reflect color
layout (location = 5) out vec4 outReflectColor;

struct Material {
    sampler2D diffuse[MAX_DIFFUSE_TEXS];
    sampler2D specular[MAX_SPECULAR_TEXS];
    sampler2D reflection[MAX_REFLECTION_MAPS];
    float shininess;
};

in VS_OUT
{
    vec2 TexCoords;
    vec4 FragWorldPos;
    vec3 WorldNormal;
} fs_in;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

uniform vec3 camPosition;

uniform Material material;

uniform int DIFFUSE_TEXS;
uniform int SPECULAR_TEXS;
uniform int REFLECTION_MAPS;

vec3 reflection_dir()
{
    vec3 incident = normalize(vec3(fs_in.FragWorldPos) - camPosition);
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
        specularValue = vec3(texture(material.specular[0], fs_in.TexCoords));
    }
    
    vec3 reflectionDir = vec3(0.0f);
    vec3 reflectionColor = vec3(0.0f);
    if (REFLECTION_MAPS > 0)
    {
        reflectionDir = reflection_dir();
        reflectionColor = vec3(texture(material.reflection[0], fs_in.TexCoords));
    }
    
    outWorldPosition = fs_in.FragWorldPos;
    outWorldNormal = vec4(normalize(fs_in.WorldNormal), 1.0f);
    outColorDiffuse = vec4(diffuseValue, 1.0f);
    outColorSpec = vec4(specularValue, 1.0f);
    outReflectDir = vec4(normalize(reflectionDir), 1.0f);
    outReflectColor = vec4(reflectionColor, 1.0f);
}
