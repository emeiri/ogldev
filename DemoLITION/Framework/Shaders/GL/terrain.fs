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

#define TERRAIN_RENDER_MODE_FULL 0
#define TERRAIN_RENDER_MODE_HEIGHT 1
#define TERRAIN_RENDER_MODE_LIGHTING_ONLY 2
#define TERRAIN_RENDER_MODE_NORMALS 3


in vec3 WorldPos;
in vec3 Normal;
in float OrigHeight;

layout(location = 0) out vec4 FragColor;

layout (binding = 1) uniform sampler2D gTexture0; // Low Elevation (e.g. Sand)
layout (binding = 2) uniform sampler2D gTexture1; // Flat Ground / Mid Elevation (e.g. Grass)
layout (binding = 3) uniform sampler2D gTexture2; // Steep Slopes / High Elevation (e.g. Rock)

uniform float gMaxTerrainHeight = 100.0f;
uniform float gLowHeightPercent = 0.25;
uniform float gHighHeightPercent = 0.75;
uniform vec3 gSunlightDir = normalize(vec3(0.0, 1.0, 0.0)); 
uniform float gAmbientFactor = 0.2f;
uniform float gTexCoordScale = 1.0/20.0;
uniform int gRenderMode = TERRAIN_RENDER_MODE_FULL;

vec4 SampleTriplanar(sampler2D tex, vec3 worldPos, vec3 normal) 
{
   // 1. Blend weights
    vec3 blend = abs(normal);
    blend = pow(blend, vec3(8.0));
    blend /= (blend.x + blend.y + blend.z);

    // 2. SCALING: Multiply world position by scale factor BEFORE derivatives
    vec3 scaledPos = worldPos * gTexCoordScale;

    // 3. DERIVATIVES: Calculate changes in world space 
    // This gives the GPU the exact, smooth scale information it needs for mipmapping
    vec3 dx = dFdx(scaledPos);
    vec3 dy = dFdy(scaledPos);

    // 4. SAMPLING: Use textureGrad to pass the custom derivatives
    vec4 xProj = textureGrad(tex, scaledPos.yz, dx.yz, dy.yz);
    vec4 yProj = textureGrad(tex, scaledPos.zx, dx.zx, dy.zx);
    vec4 zProj = textureGrad(tex, scaledPos.xy, dx.xy, dy.xy);

    // 5. Blend
    return xProj * blend.x + yProj * blend.y + zProj * blend.z;
}


void main()
{
    float gLowHeight = gMaxTerrainHeight * gLowHeightPercent;
    float gHighHeight = gMaxTerrainHeight * gHighHeightPercent;

    vec3 N = normalize(Normal);
    float SlopeFactor = smoothstep(0.65, 0.85, N.y); 
    float LowMask = smoothstep(gLowHeight - 2.0, gLowHeight + 2.0, WorldPos.y);
    float HighMask = smoothstep(gHighHeight - 5.0, gHighHeight + 5.0, WorldPos.y);

    vec4 LowColor   = SampleTriplanar(gTexture0, WorldPos, N);
    vec4 MidColor   = SampleTriplanar(gTexture1, WorldPos, N);
    vec4 HighColor  = SampleTriplanar(gTexture2, WorldPos, N);

    vec4 LowAndMid = mix(LowColor, MidColor, LowMask);
    vec4 FinalColor = mix(LowAndMid, HighColor, HighMask);

    float DiffuseFactor = max(dot(N, gSunlightDir), 0.0);
    float LightingFactor = min(gAmbientFactor + DiffuseFactor, 1.0);

    switch (gRenderMode) {
        case TERRAIN_RENDER_MODE_FULL:
            FragColor = vec4(FinalColor.rgb * LightingFactor, 1.0);
            break;
        case TERRAIN_RENDER_MODE_HEIGHT:
            FragColor = vec4(OrigHeight, OrigHeight, OrigHeight, 1.0);
            break;
        case TERRAIN_RENDER_MODE_LIGHTING_ONLY:
            FragColor = vec4(vec3(LightingFactor), 1.0);
            break;
        case TERRAIN_RENDER_MODE_NORMALS:
            FragColor = vec4(N * 0.5 + 0.5, 1.0);
            break;
    }
}
