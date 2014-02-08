#version 330

uniform sampler2D gShadowMap;

in vec2 TexCoord0;

out vec4 FragColor;

void main()
{
    float Depth = texture(gShadowMap, TexCoord0).x;
//    Depth = 1.0 - (1.0 - Depth) * 25.0;
    FragColor = vec4(Depth);
}

