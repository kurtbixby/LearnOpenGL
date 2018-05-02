#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

#define MAX_DIR_LIGHTS 1
#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 1

#define FragWorldPosition texture(worldPosition, TexCoords).xyzw
#define FragWorldNormal texture(worldNormal, TexCoords).xyz
#define FragColorDiffuse texture(colorDiffuse, TexCoords).rgb
#define FragColorSpec texture(colorSpec, TexCoords).rgb
#define FragReflectDir texture(reflectDir, TexCoords).xyz
#define FragReflectColor texture(reflectColor, TexCoords).rgb

uniform sampler2D worldPosition;
uniform sampler2D worldNormal;
uniform sampler2D colorDiffuse;
uniform sampler2D colorSpec;
uniform sampler2D reflectDir;
uniform sampler2D reflectColor;

uniform samplerCube skybox;

uniform mat4 dirLightSpaceMatrices[MAX_DIR_LIGHTS];
uniform sampler2D dirLightShadowMaps[MAX_DIR_LIGHTS];

uniform samplerCube pointLightShadowMaps_0;
uniform samplerCube pointLightShadowMaps_1;

uniform float spotFarPlane;

uniform vec3 camPosition;

uniform vec3 bloomThreshold;

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
};                                            //488 =400 + ([MAX_SPOT_LIGHT - 1] * [sizeof(SpotLight) + padding]) + sizeof(SpotLight)
//     400 + ([0] * [96]) + 88 = 400 + 88 = 488

vec3 global_lighting(Light light, sampler2D lightShadMap, mat4 lightSpaceMatrix);
vec3 point_lighting(PointLight pointLight, samplerCube lightShadMap);
vec3 spot_lighting(SpotLight spotLight);

float dir_shadow_calculation(vec4 fragLightPos, sampler2D lightShadMap);
float point_shadow_calculation(vec3 fragPos, vec3 lightPos, samplerCube lightShadMap);
//float spot_shadow_calculation(vec4 fragWorldPos, mat4 lightSpaceMat, sampler2D lightShadMap);

vec3 phong_calculation(Light light);
vec3 skybox_reflection();

void main()
{
//    vec3 fragPos = texture(worldPosition, TexCoords).rgb;
//    vec3 normal = texture(worldNormal, TexCoords).rgb;
//    vec3 diffuseValue = texture(colorDiffuse, TexCoords).rgb;
//    vec3 specularValue = texture(colorSpec, TexCoords).rgb;
    
    int i = 0;
    
    // Global Light
    vec3 global = vec3(0.0f);
    // Loop works because DIR_LIGHT is 0 or 1
    for (i = 0; i < DIR_LIGHTS; i++)
    {
        // Change shadowMap and FragLightPos for each light
        global += global_lighting(lights[i], dirLightShadowMaps[i], dirLightSpaceMatrices[i]);
    }
    
    // Point Light
    vec3 point = vec3(0.0f);
    point += point_lighting(pointLights[0], pointLightShadowMaps_0);
    point += point_lighting(pointLights[1], pointLightShadowMaps_1);
    
    vec3 spot = vec3(0.0f);
    for (i = 0; i < SPOT_LIGHTS; i++)
    {
        spot += spot_lighting(spotLights[i]);
    }
    
    vec3 reflection = skybox_reflection();
    
    vec3 result = global + point + spot + reflection;
    
    FragColor = vec4(result, 1.0f);
    
    float brightness = dot(FragColor.rgb, bloomThreshold);
    if (brightness > 1.0)
    {
        BrightColor = vec4(FragColor.rgb, 1.0f);
    }
    else
    {
        BrightColor = vec4(vec3(0.0f), 1.0f);
    }
}

// Should include a shadow map variable in the future for different lights
vec3 global_lighting(Light light, sampler2D lightShadMap, mat4 lightSpaceMatrix)
{
    vec4 fragLightPos = lightSpaceMatrix * FragWorldPosition;
    float shadowDensity = 1.0f - dir_shadow_calculation(fragLightPos, lightShadMap);
    
    vec3 ambient = 0.5f * (light.ambient * FragColorDiffuse);
    vec3 phongResult = phong_calculation(light);
    vec3 result = ambient + shadowDensity * phongResult;
    
    return result;
}

vec3 point_lighting(PointLight pointLight, samplerCube lightShadMap)
{
    vec3 viewLightPos = vec3(view * vec4(pointLight.position, 1.0f));
    
    float shadowDensity = 1.0f - point_shadow_calculation(vec3(FragWorldPosition), pointLight.position, lightShadMap);
    
    Light light = Light(-(pointLight.position - vec3(FragWorldPosition)), pointLight.ambient, pointLight.diffuse, pointLight.specular);
//    light.direction = -(pointLight.position - FragWorldPosition);
//    light.ambient = pointLight.ambient;
//    light.diffuse = pointLight.diffuse;
//    light.specular = pointLight.specular;
    vec3 phongResult = phong_calculation(light);
    
    float distance = length(pointLight.position - vec3(FragWorldPosition));
    float attenuation = 1.0f / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * distance);
    
    vec3 ambient = 0.5f * (pointLight.ambient * FragColorDiffuse);
    vec3 result = attenuation * (ambient + shadowDensity * phongResult);
    
    return result;
}

vec3 spot_lighting(SpotLight spotLight)
{
    vec3 result = vec3(0.0f);
    
    vec3 lightToFrag = vec3(FragWorldPosition) - spotLight.position;
    
    float fragDotProd = dot(normalize(lightToFrag), normalize(spotLight.direction));
    if (fragDotProd > spotLight.outerCutoff)
    {
        float shadowDensity = 1.0f;
        //        float shadowDensity = 1.0f - spot_shadow_calculation(fs_in.FragWorldPos, lightSpaceMat, lightShadMap);
        
        float intensity = 1.0f;
        if (fragDotProd < spotLight.innerCutoff)
        {
            intensity = (fragDotProd - spotLight.outerCutoff) / (spotLight.innerCutoff - spotLight.outerCutoff);
        }
        
        vec3 ambient = 0.5f * (spotLight.ambient * FragColorDiffuse);
        
        Light light = Light(-(spotLight.position - vec3(FragWorldPosition)), spotLight.ambient, spotLight.diffuse, spotLight.specular);
        //    light.direction = -(spotLight.position - FragWorldPosition);
        //    light.ambient = spotLight.ambient;
        //    light.diffuse = spotLight.diffuse;
        //    light.specular = spotLight.specular;
        vec3 phongResult = phong_calculation(light);
        
        result = intensity * (ambient + shadowDensity * phongResult);
    }
    return result;
}

float dir_shadow_calculation(vec4 fragPosLightSpace, sampler2D lightShadMap)
{
    float bias = 0.0002f;
    // Perspective division
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Map [-1, 1] -> [-0.5, 0.5] -> [0, 1]
    projCoords = projCoords * 0.5f + 0.5f;
    if (projCoords.z > 1.0f)
    {
        return 0.0f;
    }
    
    float shadowTotal = 0.0f;
    vec2 texelSize = 1.0f / textureSize(lightShadMap, 0);
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float shadMapDepth = texture(lightShadMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadowTotal += (projCoords.z - bias) > shadMapDepth ? 1.0f : 0.0f;
        }
    }
    shadowTotal /= 9.0f;
    
    return shadowTotal;
}

float point_shadow_calculation(vec3 fragPos, vec3 lightPos, samplerCube lightShadMap)
{
    vec3 lightToFrag = fragPos - lightPos;
    
    //    float bias = max(0.0005f * (1.0f - dot(normalize(fs_in.Normal), normalize(lightToFrag))), 0.00001f);
    float bias = 0.0002f;
    
    float shadowTotal = 0.0f;
    float offsetSize = 0.005f;
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            for (int z = -1; z <= 1; z++)
            {
                vec3 offsetVec = vec3(x, y, z) * offsetSize;
                float shadMapDepth = texture(lightShadMap, normalize(lightToFrag) + offsetVec).r * spotFarPlane;
                float fragDepth = length(lightToFrag);
                shadowTotal += (fragDepth - bias) > shadMapDepth ? 1.0f : 0.0f;
            }
        }
    }
    shadowTotal /= 27.0f;
    
    return shadowTotal;
}

//float spot_shadow_calculation(vec4 fragWorldPos, mat4 lightSpaceMat, sampler2D lightShadMap)
//{
//    return 0.0f;
//    float bias = 0.0002f;
//    vec4 fragLightPos = lightSpaceMat * fragWorldPos;
//    // Perspective division
//    vec3 projCoords = fragLightPos.xyz / fragLightPos.w;
//    // Map [-1, 1] -> [-0.5, 0.5] -> [0, 1]
//    projCoords = projCoords * 0.5f + 0.5f;
//    {
//        return 0.0f;
//    }
//
//    float mapDepth = texture(lightShadMap, projCoords.xy).r;
//    return (projCoords.z - bias) > mapDepth ? 1.0f : 0.0f;
//}

vec3 phong_calculation(Light light)
{
    vec3 toLight = normalize(-light.direction);
    
    vec3 norm = normalize(FragWorldNormal);
    float diff = max(dot(norm, toLight), 0.0f);
    vec3 diffuse = light.diffuse * diff * FragColorDiffuse;
    
    vec3 toCamera = normalize(camPosition - vec3(FragWorldPosition));
    vec3 reflectLight = reflect(-toLight, norm);
    float spec = pow(max(dot(toCamera, reflectLight), 0.0f), 32);
    vec3 specular = light.specular * spec * FragColorSpec;
    
    return diffuse + specular;
}

vec3 skybox_reflection()
{
    vec3 reflectionColor = texture(skybox, FragReflectDir).rgb;
    return reflectionColor * FragReflectColor;
}
