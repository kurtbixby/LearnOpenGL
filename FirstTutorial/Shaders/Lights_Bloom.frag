#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

uniform vec3 bloomThreshold;

in VS_OUT
{
    vec3 Color;
} fs_in;

void main()
{
    FragColor = vec4(fs_in.Color, 1.0f);
    
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
