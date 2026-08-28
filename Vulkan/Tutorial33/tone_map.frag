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

layout(location = 0) in vec2 TexCoords;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D gHDRSampler;

layout(push_constant) uniform PushConstants {
    int gMethodType;
    int gEnableGammaCorrection;
} pc;

// Methods of tone mapping
#define NO_TONE_MAPPING 0
#define REINHARD        1
#define BRUNO_OPSENICA  2    // https://bruop.github.io/tonemapping/
#define WITH_EXPOSURE   3

vec4 passthru()
{
    vec3 hdrColor = texture(gHDRSampler, TexCoords).rgb;

    vec4 ret = vec4(hdrColor, 1.0);

    return ret;
}

vec4 reinhard()
{             
    vec3 hdrColor = texture(gHDRSampler, TexCoords).rgb;
  
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
  
    vec4 ret = vec4(mapped, 1.0);

    return ret;
}    


float toGamma(float _r)
{
    const float gamma = 2.2;
	return pow(abs(_r), 1.0/gamma);
}


vec3 toGamma(vec3 _rgb)
{
    const float gamma = 2.2;
	return pow(abs(_rgb), vec3(1.0/gamma) );
}


vec4 toGamma(vec4 _rgba)
{
	return vec4(toGamma(_rgba.xyz), _rgba.w);
}



void main()
{
    vec4 Color;

    switch (pc.gMethodType) {
        case NO_TONE_MAPPING:
            Color = passthru();
            break;

        case REINHARD:
            Color = reinhard();
            break;
    }

    if (pc.gEnableGammaCorrection != 0) {
        FragColor = toGamma(Color);
    } else {
        FragColor = Color;
    }
}