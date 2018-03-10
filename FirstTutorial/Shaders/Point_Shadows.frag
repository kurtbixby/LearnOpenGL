#version 330 core

in vec4 FragPos;

uniform vec3 lightPos;

void main()
{
    float lightDistance = length(FragPos.xyz - lightPos);
    
    gl_FragDepth = lightDistance;
}
