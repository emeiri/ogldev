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
#define TERRAIN_RENDER_MODE_SIMPLE_TEXCOORDS 4

in vec3 WorldPos;
in vec2 TexCoords;
in vec3 Normal;
in float OrigHeight;

layout(location = 0) out vec4 FragColor;

layout (binding = 1) uniform sampler2D gTexture0;
layout (binding = 2) uniform sampler2D gTexture1;
layout (binding = 3) uniform sampler2D gTexture2;

uniform float gMaxTerrainHeight = 100.0f;
uniform float gLowHeightPercent = 0.25;
uniform float gHighHeightPercent = 0.75;
uniform vec3 gSunlightDir = normalize(vec3(0.0, 1.0, 0.0)); 
uniform float gAmbientFactor = 0.2f;
uniform float gTriplanarScale = 1.0 / 20.0; 

uniform int gRenderMode = TERRAIN_RENDER_MODE_FULL;

vec4 SampleTriplanar(sampler2D tex, vec3 worldPos, vec3 normal) 
{
    // 1. Calculate ultra-sharp blending weights
    vec3 blend = abs(normal);
    blend = pow(blend, vec3(24.0)); 
    blend /= (blend.x + blend.y + blend.z);

    // 2. Clear, proper multi-axis projection planes
    vec2 uvX = worldPos.zy * gTriplanarScale;
    vec2 uvY = worldPos.xz * gTriplanarScale;
    vec2 uvZ = worldPos.xy * gTriplanarScale;

    // 3. Calculate explicit screen-space derivatives based on the projection vectors
    vec2 dx_uvX = dFdx(uvX); vec2 dy_uvX = dFdy(uvX);
    vec2 dx_uvY = dFdx(uvY); vec2 dy_uvY = dFdy(uvY);
    vec2 dx_uvZ = dFdx(uvZ); vec2 dy_uvZ = dFdy(uvZ);

    // 4. Mipmap sharpening bias factor
    float biasFactor = 0.25;

    // 5. High-fidelity hardware gradient sampling
    vec4 xProj = textureGrad(tex, uvX, dx_uvX * biasFactor, dy_uvX * biasFactor);
    vec4 yProj = textureGrad(tex, uvY, dx_uvY * biasFactor, dy_uvY * biasFactor);
    vec4 zProj = textureGrad(tex, uvZ, dx_uvZ * biasFactor, dy_uvZ * biasFactor);

    return (xProj * blend.x) + (yProj * blend.y) + (zProj * blend.z);
}

void main()
{
    float gLowHeight = gMaxTerrainHeight * gLowHeightPercent;
    float gHighHeight = gMaxTerrainHeight * gHighHeightPercent;

    vec3 N = normalize(Normal);
    
    float LowMask    = smoothstep(gLowHeight - 2.0, gLowHeight + 2.0, WorldPos.y);
    float HighMask   = smoothstep(gHighHeight - 5.0, gHighHeight + 5.0, WorldPos.y);

    vec4 LowColor;
    vec4 MidColor;
    vec4 HighColor;

    // Clean, original height distribution logic (No rogue snow textures)
    if (gRenderMode == TERRAIN_RENDER_MODE_SIMPLE_TEXCOORDS) {
        LowColor  = texture(gTexture0, TexCoords);
        MidColor  = texture(gTexture1, TexCoords);
        HighColor = texture(gTexture2, TexCoords);
    } else {
        LowColor  = SampleTriplanar(gTexture0, WorldPos, N);
        MidColor  = SampleTriplanar(gTexture1, WorldPos, N);
        HighColor = SampleTriplanar(gTexture2, WorldPos, N);
    }

    vec4 LowAndMid = mix(LowColor, MidColor, LowMask);
    vec4 FinalColor = mix(LowAndMid, HighColor, HighMask);

    float DiffuseFactor = max(dot(N, gSunlightDir), 0.0);
    float LightingFactor = min(gAmbientFactor + DiffuseFactor, 1.0);

    switch (gRenderMode) {
        case TERRAIN_RENDER_MODE_FULL:
        case TERRAIN_RENDER_MODE_SIMPLE_TEXCOORDS:
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
