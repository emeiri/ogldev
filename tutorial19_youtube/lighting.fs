#version 330

in vec2 TexCoord0;

out vec4 FragColor;

struct BaseLight
{
    vec3 Color;
    float AmbientIntensity;
};

uniform BaseLight gLight;
uniform sampler2D gSampler;

void main()
{
    FragColor = texture2D(gSampler, TexCoord0.xy) *
                vec4(gLight.Color, 1.0f) *
                gLight.AmbientIntensity;
}
