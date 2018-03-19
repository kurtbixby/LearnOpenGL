#version 330 core

uniform float farPlane;
uniform vec2 center;

void main()
{
    float distance = length(gl_FragCoord.xy - center);
    if (distance > center.x)
    {
        gl_FragDepth = 1.0f;
    }
    else
    {
        gl_FragDepth = gl_FragCoord.z;
    }
}
