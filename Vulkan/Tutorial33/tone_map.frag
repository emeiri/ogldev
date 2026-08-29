/*

        Copyright 2026 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#version 460

// Methods of tone mapping
#define NO_TONE_MAPPING 0
#define REINHARD        1


layout(location = 0) in vec2 TexCoords;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D gHDRSampler;

layout(push_constant) uniform PushConstants {
    int gMethodType;
} pc;


vec4 passthru()
{
    vec3 hdrColor = texture(gHDRSampler, TexCoords).rgb;

    vec4 ret = vec4(hdrColor * 2, 1.0);

    return ret;
}

vec4 reinhard(vec3 hdrColor)
{              
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
  
    vec4 ret = vec4(mapped, 1.0);

    return ret;
}    


vec4 ReinhardExtended(vec3 color, float W) 
{
    vec3 numerator = color * (1.0f + (color / (W * W)));
    vec3 mapped = numerator / (1.0f + color);
    return vec4(mapped, 1.0f);
}



vec4 ACESFilm(vec3 x) 
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    vec3 mapped = clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
    return vec4(mapped, 1.0f);
}


void main()
{
    vec4 Color;

    switch (pc.gMethodType) {
        case NO_TONE_MAPPING:
            Color = passthru();
            break;

        case REINHARD:
            vec3 hdrColor = texture(gHDRSampler, TexCoords).rgb;
            Color = reinhard(hdrColor);
            //Color = ACESFilm(hdrColor);
            //Color = ReinhardExtended(hdrColor, 1.0f);
            break;
    }

    FragColor = Color;
}