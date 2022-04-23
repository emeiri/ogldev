#version 410

layout(location = 0) out vec4 FragColor;

uniform sampler2D gSampler;

in vec2 TexCoords0;

void main()
{
    FragColor = texture2D(gSampler, TexCoords0.xy);

    if (FragColor == vec4(0.0)) {
        discard;
    }
}
