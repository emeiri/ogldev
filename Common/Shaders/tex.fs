#version 410

layout(location = 0) out vec4 FragColor;

uniform sampler2D gSampler;

in vec2 TexCoords0;

void main()
{
    FragColor = texture(gSampler, TexCoords0.xy);
}
