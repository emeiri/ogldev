#version 330

layout(location = 0) out vec4 FragColor;

in vec2 Tex;
in vec3 Normal;

uniform sampler2D gTextureHeight0;

uniform vec3 gReversedLightDir;

void main()
{
    vec4 TexColor = texture(gTextureHeight0, Tex);

    vec3 Normal_ = normalize(Normal);

    float Diffuse = dot(Normal_, gReversedLightDir);

    FragColor = TexColor * Diffuse;
}
