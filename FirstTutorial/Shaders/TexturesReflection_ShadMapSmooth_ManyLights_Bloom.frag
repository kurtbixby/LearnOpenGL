#version 330 core

#define MAX_DIFFUSE_TEXS 1
#define MAX_SPECULAR_TEXS 1
#define MAX_REFLECTION_MAPS 1

#define MAX_DIR_LIGHTS 1
#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 1

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct Material {
	sampler2D diffuse[MAX_DIFFUSE_TEXS];
	sampler2D specular[MAX_SPECULAR_TEXS];
	sampler2D reflection[MAX_REFLECTION_MAPS];
	float shininess;
};

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

in VS_OUT
{
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoords;
    vec4 FragWorldPos;
} fs_in;

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

uniform Material material;

uniform int DIFFUSE_TEXS;
uniform int SPECULAR_TEXS;
uniform int REFLECTION_MAPS;

uniform mat4 dirLightSpaceMatrices[MAX_DIR_LIGHTS];
//uniform mat4 pointLightSpaceMatrices[MAX_POINT_LIGHTS];
//uniform mat4 spotLightSpaceMatrices[MAX_SPOT_LIGHTS];

uniform sampler2D dirLightShadowMaps[MAX_DIR_LIGHTS];

//uniform samplerCube pointLightShadowMaps[MAX_POINT_LIGHTS];
uniform samplerCube pointLightShadowMaps_0;
uniform samplerCube pointLightShadowMaps_1;
//uniform samplerCube pointLightShadowMaps_2;
//uniform samplerCube pointLightShadowMaps_3;

//uniform sampler2D spotLightShadowMaps[MAX_SPOT_LIGHTS];

uniform samplerCube skybox;

uniform float spotFarPlane;

uniform vec3 bloomThreshold;

float specular_coefficient(vec3 viewDir, vec3 toLightDir, vec3 normal, int shininess)
{
    vec3 normdView = normalize(viewDir);
    vec3 normdLight = normalize(toLightDir);
    vec3 halfway = normalize(normdView + normdLight);
    return pow(max(dot(normal, halfway), 0.0f), shininess);
}

float dir_shadow_calculation(vec4 fragLightPos, sampler2D lightShadMap, vec3 lightDir)
{
//    float bias = max(0.0005f * (1.0f - dot(normalize(fs_in.Normal), normalize(lightDir))), 0.00005f);
    float bias = 0.0002f;
    // Perspective division
    vec3 projCoords = fragLightPos.xyz / fragLightPos.w;
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

float spot_shadow_calculation(vec4 fragWorldPos, mat4 lightSpaceMat, sampler2D lightShadMap)
{
    return 0.0f;
    float bias = 0.0002f;
    vec4 fragLightPos = lightSpaceMat * fragWorldPos;
    // Perspective division
    vec3 projCoords = fragLightPos.xyz / fragLightPos.w;
    // Map [-1, 1] -> [-0.5, 0.5] -> [0, 1]
    projCoords = projCoords * 0.5f + 0.5f;
    {
        return 0.0f;
    }
    
    float mapDepth = texture(lightShadMap, projCoords.xy).r;
    return (projCoords.z - bias) > mapDepth ? 1.0f : 0.0f;
}

// Should include a shadow map variable in the future for different lights
vec3 global_lighting(Light light, sampler2D lightShadMap, mat4 lightSpaceMatrix, vec3 diffuseValue, vec3 specularValue)
{
    vec4 fragLightPos = lightSpaceMatrix * fs_in.FragWorldPos;
    float shadowDensity = 1.0f - dir_shadow_calculation(fragLightPos, lightShadMap, light.direction);
    
    vec3 ambient = 0.5f * (light.ambient * diffuseValue);

	vec3 viewLightDir = normalize(-vec3(view * vec4(light.direction, 0.0f)));
	vec3 norm = normalize(fs_in.Normal);
	float diff = max(dot(norm, viewLightDir), 0.0f);
	vec3 diffuse = light.diffuse * diff * diffuseValue;

	vec3 viewDir = normalize(-fs_in.FragPos);
	vec3 reflectDir = reflect(-viewLightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = light.specular * spec * specularValue;
	vec3 result = ambient + shadowDensity * (diffuse + specular);

	return result * shadowDensity;
}

vec3 point_lighting(PointLight pointLight, samplerCube lightShadMap, vec3 diffuseValue, vec3 specularValue)
{
    vec3 viewLightPos = vec3(view * vec4(pointLight.position, 1.0f));

    float shadowDensity = 1.0f - point_shadow_calculation(vec3(fs_in.FragWorldPos), pointLight.position, lightShadMap);

    vec3 ambient = 0.5f * (pointLight.ambient * diffuseValue);

    vec3 norm = normalize(fs_in.Normal);
    vec3 lightDir = normalize(viewLightPos - fs_in.FragPos);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = pointLight.diffuse * diff * diffuseValue;

    vec3 viewDir = normalize(-fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = pointLight.specular * spec * specularValue;

    float distance = length(viewLightPos - fs_in.FragPos);
    float attenuation = 1.0f / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * distance);

    vec3 result = attenuation * (ambient + shadowDensity * (diffuse + specular));

    return result;
}

vec3 spot_lighting(SpotLight spotLight, vec3 diffuseValue, vec3 specularValue)
{
    vec3 result = vec3(0.0f);

    vec3 viewLightPos = vec3(view * vec4(spotLight.position, 1.0f));
    vec3 lightDir = normalize(viewLightPos - fs_in.FragPos);

    float fragDotProd = dot(-lightDir, normalize(vec3(view * vec4(spotLight.direction, 0.0f))));
    if (fragDotProd > spotLight.outerCutoff)
    {
        float shadowDensity = 1.0f;
//        float shadowDensity = 1.0f - spot_shadow_calculation(fs_in.FragWorldPos, lightSpaceMat, lightShadMap);

        float intensity = 1.0f;

        if (fragDotProd < spotLight.innerCutoff)
        {
            intensity = (fragDotProd - spotLight.outerCutoff) / (spotLight.innerCutoff - spotLight.outerCutoff);
        }

        vec3 ambient = spotLight.ambient * diffuseValue;

        vec3 norm = normalize(fs_in.Normal);
        float diff = max(dot(norm, lightDir), 0.0f);
        vec3 diffuse = spotLight.diffuse * diff * diffuseValue;

        vec3 viewDir = normalize(-fs_in.FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
        vec3 specular = spotLight.specular * spec * specularValue;

        result = intensity * (ambient + shadowDensity * (diffuse + specular));
    }
    return result;
}

vec3 skybox_reflection()
{	
	vec3 incident = normalize(fs_in.FragPos);
	vec3 norm = normalize(fs_in.Normal);
	vec3 reflected = reflect(incident, norm);
	// this feels ugly and requires excess processing?
	reflected = vec3(inverse(view) * vec4(reflected, 0.0f));

	vec3 reflectionColor = texture(skybox, reflected).rgb;
	vec3 reflectionIntensity = texture(material.reflection[0], fs_in.TexCoords).rgb;
	return reflectionColor * reflectionIntensity;
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
	
	vec3 reflection = vec3(0.0f);
	if (REFLECTION_MAPS > 0)
	{
		reflection = skybox_reflection();
	}

    int i = 0;

    // Global Light
    vec3 global = vec3(0.0f);
    // Loop works because DIR_LIGHT is 0 or 1
    for (i = 0; i < DIR_LIGHTS; i++)
    {
        // Change shadowMap and FragLightPos for each light
        global += global_lighting(lights[i], dirLightShadowMaps[i], dirLightSpaceMatrices[i], diffuseValue, specularValue);
    }

    // Point Light
    vec3 point = vec3(0.0f);
    
    point += point_lighting(pointLights[0], pointLightShadowMaps_0, diffuseValue, specularValue);
    point += point_lighting(pointLights[1], pointLightShadowMaps_1, diffuseValue, specularValue);
    
//    if (pointLightShadowMaps_2.env_mode != TEXMODE_OFF)
//    {
//        point += point_lighting(pointLights[2], pointLightShadowMaps_2, diffuseValue, specularValue);
//    }
//    if (pointLightShadowMaps_3.env_mode != TEXMODE_OFF)
//    {
//        point += point_lighting(pointLights[3], pointLightShadowMaps_3, diffuseValue, specularValue);
//    }
    
    // What I want to do
//    for (i = 0; i < POINT_LIGHTS; i++)
//    {
//        point += point_lighting(pointLights[i], pointLightShadowMaps[i], diffuseValue, specularValue);
//    }

    // Spot Light
    vec3 spot = vec3(0.0f);
    for (i = 0; i < SPOT_LIGHTS; i++)
    {
        spot += spot_lighting(spotLights[i], diffuseValue, specularValue);
    }

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
