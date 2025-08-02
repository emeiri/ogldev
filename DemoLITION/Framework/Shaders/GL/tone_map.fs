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

#version 460

in vec2 TexCoords;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D gHDRSampler;

layout(std430, binding = 1) buffer Result {
    float Exposure[]; // One per workgroup
};


uniform float gAvgLum;
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


vec4 with_exposure_tiled()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(gHDRSampler, TexCoords).rgb;

    int x = int(TexCoords.x * 192.0);
    int y = int(TexCoords.y * 108.0);
    float Exposure = Exposure[y * 192 + x];
  
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * Exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    vec4 ret = vec4(mapped, 1.0);

 //  vec4 ret = vec4(Exposure);

    return ret;
}  


vec4 new_method()
{
    vec4 hdrColor = texture(gHDRSampler, TexCoords);
    float lum = dot(hdrColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    float mappedL = gExposure * lum / max(gAvgLum, 0.1);
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


vec3 convertRGB2XYZ(vec3 _rgb)
{
	// Reference(s):
	// - RGB/XYZ Matrices
	//   https://web.archive.org/web/20191027010220/http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
	vec3 xyz;
	xyz.x = dot(vec3(0.4124564, 0.3575761, 0.1804375), _rgb);
	xyz.y = dot(vec3(0.2126729, 0.7151522, 0.0721750), _rgb);
	xyz.z = dot(vec3(0.0193339, 0.1191920, 0.9503041), _rgb);
	return xyz;
}

vec3 convertXYZ2RGB(vec3 _xyz)
{
	vec3 rgb;
	rgb.x = dot(vec3( 3.2404542, -1.5371385, -0.4985314), _xyz);
	rgb.y = dot(vec3(-0.9692660,  1.8760108,  0.0415560), _xyz);
	rgb.z = dot(vec3( 0.0556434, -0.2040259,  1.0572252), _xyz);
	return rgb;
}


vec3 convertXYZ2Yxy(vec3 _xyz)
{
	// Reference(s):
	// - XYZ to xyY
	//   https://web.archive.org/web/20191027010144/http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_xyY.html
	float inv = 1.0/dot(_xyz, vec3(1.0, 1.0, 1.0) );
	return vec3(_xyz.y, _xyz.x*inv, _xyz.y*inv);
}


vec3 convertYxy2XYZ(vec3 _Yxy)
{
	// Reference(s):
	// - xyY to XYZ
	//   https://web.archive.org/web/20191027010036/http://www.brucelindbloom.com/index.html?Eqn_xyY_to_XYZ.html
	vec3 xyz;
	xyz.x = _Yxy.x*_Yxy.y/_Yxy.z;
	xyz.y = _Yxy.x;
	xyz.z = _Yxy.x*(1.0 - _Yxy.y - _Yxy.z)/_Yxy.z;
	return xyz;
}


vec3 convertRGB2Yxy(vec3 _rgb)
{
	return convertXYZ2Yxy(convertRGB2XYZ(_rgb) );
}

vec3 convertYxy2RGB(vec3 _Yxy)
{
	return convertXYZ2RGB(convertYxy2XYZ(_Yxy) );
}

float Reinhard2(float x, float White) {
    //const float L_white = 4.0;
    //return (x * (1.0 + x / (L_white * L_white))) / (1.0 + x);
    float ret = (x * (1.0 + x / (White * White))) / (1.0 + x);

    return ret;
}

float toGamma(float _r)
{
	return pow(abs(_r), 1.0/2.2);
}

vec3 toGamma(vec3 _rgb)
{
	return pow(abs(_rgb), vec3(1.0/2.2) );
}

vec4 toGamma(vec4 _rgba)
{
	return vec4(toGamma(_rgba.xyz), _rgba.w);
}

vec4 bruno()
{
    vec3 rgb = texture2D(gHDRSampler, TexCoords).rgb;
   // float avgLum = texture2D(s_texAvgLum, v_texcoord0).r;

    // Yxy.x is Y, the luminance
    vec3 Yxy = convertRGB2Yxy(rgb);

    float lp = Yxy.x / (9.6 * gAvgLum + 0.0001);

    // Replace this line with other tone mapping functions
    // Here we applying the curve to the luminance exclusively
    Yxy.x = Reinhard2(lp, White);

    rgb = convertYxy2RGB(Yxy);

    vec4 ret = toGamma(vec4(rgb, 1.0) );

    return ret;
}


void main()
{
   // FragColor = reinhard();
    //FragColor = new_method();
    //FragColor = with_exposure_tiled();
    //FragColor = with_exposure();
    FragColor = bruno();
}