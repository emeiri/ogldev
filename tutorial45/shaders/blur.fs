#version 330

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D gColorMap;

void main()
{
    vec3 Color = texture(gColorMap, TexCoord).xyz;

    FragColor = vec4(Color, 1.0);
}