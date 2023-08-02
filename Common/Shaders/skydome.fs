#version 410

layout(location = 0) out vec4 FragColor;

uniform sampler2D gSampler;
uniform vec4 gLowColor = vec4(0.6, 0.38, 0.66, 1.0);
uniform vec4 gHighColor = vec4(0.0, 0.15, 0.66, 1.0);

in vec2 TexCoords0;
in float Height;

void main()
{
      vec4 TexColor = texture2D(gSampler, TexCoords0.xy);

      float H = Height;

      if (H < 0.0) {
          H = 0.0;
      }

      vec4 SkyColor = mix(gLowColor, gHighColor, H);

      FragColor = TexColor;// * SkyColor;
}
