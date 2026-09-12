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

// Methods of tone mapping
#define NO_TONE_MAPPING 0
#define REINHARD        1
#define BRUNO_OPSENICA  2    // https://bruop.github.io/tonemapping/
#define WITH_EXPOSURE   3


uniform float gAvgLum;
uniform float gExposure = 0.4457;
uniform float White = 1.0;
uniform int gMethodType = 0;
uniform bool gEnableGammaCorrection = true;

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


vec4 with_exposure()
{             
    vec3 hdrColor = texture(gHDRSampler, TexCoords).rgb;
  
    float exposure = gExposure;

    const float targetGray = 0.18;

    float bias = 0.03;
    float minLum = 0.01;
    float maxLum = 1.0;
   
    //float exposure = targetGray / clamp(gAvgLum + bias, minLum, maxLum);
        
    // Reinhard operator
    vec3 mapped = hdrColor * exposure;
    mapped = mapped / (mapped + vec3(1.0));
  
    vec4 ret = vec4(mapped, 1.0);

    return ret;
}  


vec3 RRTAndODTFit(vec3 v) {
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}


vec4 toneMapACES() 
{
    vec3 hdrColor = texture(gHDRSampler, TexCoords).rgb;

    hdrColor = RRTAndODTFit(hdrColor);
    vec4 ret = vec4(clamp(hdrColor, 0.0, 1.0), 1.0);

    return ret;
}


vec4 with_exposure_tiled()
{                 
    vec3 hdrColor = texture(gHDRSampler, TexCoords).rgb;

    int x = int(TexCoords.x * 192.0);
    int y = int(TexCoords.y * 108.0);
    float Exposure = Exposure[y * 192 + x];
  
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * Exposure);
  
    vec4 ret = vec4(mapped, 1.0);

    return ret;
}  


vec4 new_method()
{
    // Retrieve high-res color from texture
    vec4 color = texture(gHDRSampler, TexCoords);
    
    // Convert to XYZ
    vec3 xyzCol = rgb2xyz * vec3(color);

    // Convert to xyY
    float xyzSum = xyzCol.x + xyzCol.y + xyzCol.z;
    vec3 xyYCol = vec3( xyzCol.x / xyzSum, xyzCol.y / xyzSum, xyzCol.y);

    // Apply the tone mapping operation to the luminance (xyYCol.z or xyzCol.y)
    float L = (gExposure * xyYCol.z) / gAvgLum;
    L = (L * ( 1 + L / (White * White) )) / ( 1 + L );

    // Using the new luminance, convert back to XYZ
    xyzCol.x = (L * xyYCol.x) / (xyYCol.y);
    xyzCol.y = L;
    xyzCol.z = (L * (1 - xyYCol.x - xyYCol.y))/xyYCol.y;

    // Convert back to RGB and send to output buffer
    vec4 ret = vec4( xyz2rgb * xyzCol, 1.0);

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


float Reinhard2(float x, float White) 
{
    //const float L_white = 4.0;
    //return (x * (1.0 + x / (L_white * L_white))) / (1.0 + x);
    float ret = (x * (1.0 + x / (White * White))) / (1.0 + x);

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


// Based on: https://bruop.github.io/tonemapping/
vec4 bruno_opsenica()
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
    vec4 Color;

    switch (gMethodType) {
        case NO_TONE_MAPPING:
            Color = passthru();
            break;

        case REINHARD:
            Color = reinhard();
            break;

        case BRUNO_OPSENICA:
            Color = bruno_opsenica();
            break;

        case WITH_EXPOSURE:
            Color = with_exposure();
            //Color = new_method();
            //Color = toneMapACES();
            break;
    }

    if (gEnableGammaCorrection) {
        FragColor = toGamma(Color);
    } else {
        FragColor = Color;
    }
    
    //
    //
    //FragColor = with_exposure_tiled();        
}