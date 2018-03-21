#version 330 core

#define MAX_LIGHTS 2

struct Light {      // 60 bytes needs 4 bytes of padding
    vec3 position; //  0
    
    vec3 ambient;   // 16
    vec3 diffuse;   // 32
    vec3 specular;  // 48
};

out vec4 FragColor;

in VS_OUT
{
    vec2 TexCoords;
    mat3 InvTBN;
    vec3 TBNViewPos;
    vec3 TBNFragPos;
} fs_in;

in TBN_Lighting
{
    Light lights[MAX_LIGHTS];
} tbn_lighting;

uniform sampler2D diffuse[1];
uniform sampler2D normals[1];

vec3 light_calculation(vec3 colorValue, Light light, vec3 normal)
{
    float ambiCoef = 1.0f;
    vec3 ambient = ambiCoef * light.ambient * colorValue;
    
    vec3 lightDir = normalize(light.position - fs_in.TBNFragPos);
    float diffCoef = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = diffCoef * light.diffuse * colorValue;
    
    vec3 viewDir = fs_in.TBNViewPos - fs_in.TBNFragPos;
    vec3 reflectDir = reflect(-lightDir, normal);
    float specCoef = pow(max(dot(viewDir, reflectDir), 0.0f), 16);
    vec3 specular = specCoef * light.specular * colorValue;
    
    float distance = length(light.position - fs_in.TBNFragPos);
    float attenuation = 1.0f;// / (1.0f + 0.09f * distance + 0.032f * distance);
    
    vec3 result = attenuation * (ambient + diffuse + specular);
    
    return result;
}

void main()
{
    vec3 normal = vec3(texture(normals[0], fs_in.TexCoords));
    normal = normalize(normal * 2.0f - 1.0f);
    
    vec3 diffuseValue = vec3(texture(diffuse[0], fs_in.TexCoords));
    
    vec3 result = vec3(0.0f);
    int i;
    for (i = 0; i < MAX_LIGHTS; i++)
    {
        result += light_calculation(diffuseValue, tbn_lighting.lights[i], normal);
    }
    
    FragColor = vec4(result, 1.0f);
}
