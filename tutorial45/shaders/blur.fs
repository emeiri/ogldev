#version 330

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D gColorMap;

float Offsets[4] = float[]( -1.5, -0.5, 0.5, 1.5 );

void main()
{
    vec3 Color = vec3(0.0, 0.0, 0.0);

    for (int i = 0 ; i < 4 ; i++) {
        for (int j = 0 ; j < 4 ; j++) {
            vec2 tc = TexCoord;
            tc.x = TexCoord.x + Offsets[j] / textureSize(gColorMap, 0).x;
            tc.y = TexCoord.y + Offsets[i] / textureSize(gColorMap, 0).y;
            Color += texture(gColorMap, tc).xyz;
        }
    }

    Color /= 16.0;

    FragColor = vec4(Color, 1.0);
}