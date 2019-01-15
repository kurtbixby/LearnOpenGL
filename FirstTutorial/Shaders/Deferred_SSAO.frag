#version 330 core

#define MAX_SAMPLES_NUMBER 64

layout (location = 0) out vec4 outOcclusion;

in vec2 TexCoords;

uniform sampler2D viewPosition;
uniform sampler2D viewNormal;
uniform sampler2D rotationNoise;

uniform int numSamples;
uniform vec3 sampleOffsets[MAX_SAMPLES_NUMBER];
uniform float ssaoRadius;

float bias = 0.025f;
const vec2 noiseScale = vec2(1600.0/4.0, 1200.0/4.0);

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

mat3 create_tbn_matrix();

void main()
{
//    outOcclusion = vec4(vec3(texture(viewPosition, TexCoords).xyz + texture(viewNormal, TexCoords).xyz), 1.0f);
//    return;
    
    vec3 viewFragPos = texture(viewPosition, TexCoords).xyz;
    mat3 TBN = create_tbn_matrix();
    float occlusion = 0.0f;
    int outOfRangeSamples = 0;
    for (int i = 0; i < numSamples; i++)
    {
        vec3 sampleOffset = TBN * sampleOffsets[i];
        vec3 viewSamplePos = viewFragPos + sampleOffset * ssaoRadius;
        
        vec4 offsetVector = projection * vec4(viewSamplePos, 1.0f);
        offsetVector.xyz = offsetVector.xyz / offsetVector.w;
        offsetVector.xyz = offsetVector.xyz * 0.5f + 0.5f;
        
        if (offsetVector.x < 0.0f || offsetVector.x > 1.0f || offsetVector.y < 0.0f || offsetVector.y > 1.0f)
        {
            outOfRangeSamples++;
            continue;
        }
        
        float offsetSampleDepth = texture(viewPosition, offsetVector.xy).z;
        float distanceCoef = smoothstep(0.0f, 1.0f, ssaoRadius / abs(viewFragPos.z - offsetSampleDepth));
        occlusion += ((offsetSampleDepth >= viewSamplePos.z + bias ? 1.0f : 0.0f) * distanceCoef);
    }
    
    occlusion = 1.0f - (occlusion / (numSamples - outOfRangeSamples));
    outOcclusion = vec4(vec3(occlusion), 1.0f);
}

mat3 create_tbn_matrix()
{
    vec3 normal = texture(viewNormal, TexCoords).xyz;
    vec3 rotationVec = texture(rotationNoise, TexCoords * noiseScale).xyz;
    float scalerProject = dot(rotationVec, normal);// / length(normal);
    // Works as long as rotationVec.xy and normal.xy aren't parallel
    vec3 tangent = normalize(rotationVec - normal * scalerProject);
    vec3 bitan = cross(tangent, normal);
    mat3 TBN = mat3(tangent, bitan, normal);
    return TBN;
}
