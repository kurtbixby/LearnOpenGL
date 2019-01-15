#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceMatrix;

#define MAX_DIR_LIGHTS 1
#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 1

struct Light {      // 60 bytes needs 4 bytes of padding
    vec3 direction; //  0
    
    vec3 ambient;   // 16
    vec3 diffuse;   // 32
    vec3 specular;  // 48
};

// Make PointLight contain a Light?
struct PointLight {     // 72 bytes needs 8 bytes of padding
    vec3 position;      //  0
    
    vec3 ambient;       // 16
    vec3 diffuse;       // 32
    vec3 specular;      // 48
    
    float constant;     // 60
    float linear;       // 64
    float quadratic;    // 68
};

struct SpotLight {      // 84 bytes needs 12 bytes of padding
    vec3 position;      //  0
    vec3 direction;     // 16
    
    vec3 ambient;       // 32
    vec3 diffuse;       // 48
    vec3 specular;      // 64
    
    float innerCutoff;  // 76
    float outerCutoff;  // 80
};

out VS_OUT
{
    vec3 Color;
} vs_out;

layout (std140) uniform Matrices
{
	mat4 projection;
	mat4 view;
};

layout (std140) uniform Lighting
{
    int DIR_LIGHTS;                           //  0
    int POINT_LIGHTS;                         //  4
    int SPOT_LIGHTS;                          //  8
    //    int padding;                              // 12
    Light lights[MAX_DIR_LIGHTS];             // 16
    PointLight pointLights[MAX_POINT_LIGHTS]; // 80 = 16 + ([MAX_DIR_LIGHT - 1] * [sizeof(Light) + padding]) + sizeof(Light) + alignment = 16 + 0 + 64 + 0
    SpotLight spotLights[MAX_SPOT_LIGHTS];    //400 = 80 + ([MAX_POINT_LIGHT - 1] * [sizeof(PointLight) + padding]) + sizeof(PointLight) + alignment
    //      80 + ([3] * [80]) + 76) + 4 = 80 + 316 + 4 = 400
};

void main()
{
    gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0f);
    vs_out.Color = pointLights[gl_InstanceID].specular;
}
