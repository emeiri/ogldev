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

#version 460 core

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

layout(location = 0) out vec4 FragColor;

layout (binding = 1) uniform sampler2D gTexture0; // Low Elevation (e.g. Sand)
layout (binding = 2) uniform sampler2D gTexture1; // Flat Ground / Mid Elevation (e.g. Grass)
layout (binding = 3) uniform sampler2D gTexture2; // Steep Slopes / High Elevation (e.g. Rock)

uniform float gMaxTerrainHeight = 100.0f;
uniform float gLowHeightPercent = 0.25;
uniform float gHighHeightPercent = 0.75;
uniform vec3 gSunlightDir = normalize(vec3(0.0, 1.0, 0.0)); 
uniform float gAmbientFactor = 0.2f;

vec4 SampleTriplanar(sampler2D tex, vec3 worldPos, vec3 normal, float scale) 
{
    // 1. Calculate blend weights from the absolute normal
    // Use the absolute value because sign doesn't determine the projection axis
    vec3 blend = abs(normal);
    
    // Sharpen the blend to reduce blurring on 45-degree slopes
    blend = pow(blend, vec3(8.0)); 
    
    // Ensure weights sum to 1.0 to maintain consistent brightness
    blend /= (blend.x + blend.y + blend.z);

    // 2. Sample the texture from 3 directions
    // Projection UVs are derived from the other two axes
    vec4 xProj = texture(tex, worldPos.yz * scale); // Side
    vec4 yProj = texture(tex, worldPos.zx * scale); // Top/Bottom
    vec4 zProj = texture(tex, worldPos.xy * scale); // Front/Back

    // 3. Blend the samples together
    return xProj * blend.x + yProj * blend.y + zProj * blend.z;
}


void main()
{
    vec2 DetailUV = TexCoords * 8; 
    float tiling = 0.1; // Adjust for your world scale

    float gLowHeight = gMaxTerrainHeight * gLowHeightPercent;
    float gHighHeight = gMaxTerrainHeight * gHighHeightPercent;

    vec3 N = normalize(Normal);
    float SlopeFactor = smoothstep(0.65, 0.85, N.y); 
    float LowMask = smoothstep(gLowHeight - 2.0, gLowHeight + 2.0, WorldPos.y);
    float HighMask = smoothstep(gHighHeight - 5.0, gHighHeight + 5.0, WorldPos.y);

    vec4 LowColor   = SampleTriplanar(gTexture0, WorldPos, N, tiling);
    vec4 MidColor   = SampleTriplanar(gTexture1, WorldPos, N, tiling);
    vec4 HighColor  = SampleTriplanar(gTexture2, WorldPos, N, tiling);

    vec4 LowAndMid = mix(LowColor, MidColor, LowMask);
    vec4 FinalColor = mix(LowAndMid, HighColor, HighMask);

    float DiffuseFactor = max(dot(N, gSunlightDir), 0.0);
    float LightingFactor = min(gAmbientFactor + DiffuseFactor, 1.0);

    // Final color multiplication
    FragColor = vec4(FinalColor.rgb * LightingFactor, 1.0);

   //FragColor = vec4(vec3(WorldPos.y / gMaxTerrainHeight.0), 1.0);   
}
