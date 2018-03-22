#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float kernel[9];
uniform float gamma;
uniform float exposure;

const float offset = 1.0 / 300.0;

void main()
{
	vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );

	vec3 samples[9];
	for (int i = 0; i < 9; i++)
	{
		samples[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]).rgb);
	}

	vec3 color = vec3(0.0f);
	for (int i = 0; i < 9; i++)
	{
		color += samples[i] * kernel[i];
	}
    vec3 mapped = vec3(1.0) - exp(-color * exposure);
    
    vec3 finalColor = pow(mapped, vec3(1.0f/gamma));
    FragColor = vec4(finalColor , 1.0f);
}
