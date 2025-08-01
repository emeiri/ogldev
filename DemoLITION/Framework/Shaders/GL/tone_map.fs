/*

        Copyright 2025 Etay Meiri

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

#version 420

in vec2 TexCoords;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D gHDRSampler;

uniform float AveLum;
uniform float gExposure = 0.4457;
uniform float White = 1.0;
uniform bool DoToneMap = true;

// XYZ/RGB conversion matrices from:
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html


uniform mat3 rgb2xyz = mat3(
    0.4124564, 0.2126729, 0.0193339,
    0.3575761, 0.7151522, 0.1191920,
    0.1804375, 0.0721750, 0.9503041 );

uniform mat3 xyz2rgb = mat3(
    3.2404542, -0.9692660, 0.0556434,
    -1.5371385,  1.8760108, -0.2040259,
    -0.4985314,  0.0415560,  1.0572252 );


vec4 reinhard()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(gHDRSampler, TexCoords).rgb;
  
    // reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    vec4 ret = vec4(mapped, 1.0);

    return ret;
}    

vec4 with_exposure()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(gHDRSampler, TexCoords).rgb;
  
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * gExposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    vec4 ret = vec4(mapped, 1.0);

    return ret;
}  


vec4 new_method()
{
    vec4 hdrColor = texture(gHDRSampler, TexCoords);
float lum = dot(hdrColor.rgb, vec3(0.2126, 0.7152, 0.0722));
float mappedL = gExposure * lum / max(AveLum, 0.1);
mappedL = (mappedL * (1.0 + mappedL / (White * White))) / (1.0 + mappedL);

// Scale RGB to maintain color ratios
vec3 color = hdrColor.rgb * mappedL / lum; // preserve chromaticity
color = pow(color, vec3(1.0 / 2.2));       // gamma correction

vec4 ret = vec4(clamp(color, 0.0, 1.0), 1.0);
    // Output
    //FragColor = vec4(DoToneMap ? xyz2rgb * mappedXYZ : hdrColor.rgb, 1.0);

 // FragColor = vec4(hdrColor.rgb, 1.0);
// FragColor = vec4(hdrColor.rgb / (vec3(1.0) + hdrColor.rgb),1.0);
return ret;
}


void main()
{
    //FragColor = reinhard();
    //FragColor = new_method();;
    FragColor = with_exposure();
}