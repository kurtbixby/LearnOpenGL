#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D ssaoTexture;
uniform int blurSize;

void main()
{
    int blurSizeHalf = blurSize / 2;
    float totalColor = 0.0f;
    vec2 texelSize = 1.0f / vec2(textureSize(ssaoTexture, 0));
    for (int x = -blurSizeHalf; x < blurSizeHalf; x++)
    {
        for (int y = -blurSizeHalf; y < blurSizeHalf; y++)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            totalColor += texture(ssaoTexture, TexCoords + offset).r;
        }
    }
    totalColor = totalColor / (blurSize * blurSize);
    FragColor = vec4(vec3(totalColor), 1.0f);
}
