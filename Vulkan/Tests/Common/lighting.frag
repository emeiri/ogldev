/*

        Copyright 2024 Etay Meiri

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

#extension GL_EXT_nonuniform_qualifier : require

#define LIGHTING_MODE_UNLIT 0
#define LIGHTING_MODE_NORMALS 1
#define LIGHTING_MODE_AMBIENT_ONLY 2
#define LIGHTING_MODE_FULL 3

#define MAX_NUM_LIGHTS 16

struct LightSource {
    vec3 Color;                    // offset 0
    int LightType;                 // offset 12
    vec3 Direction;                // offset 16
    float AmbientIntensity;        // offset 28
    vec3 WorldPos;                 // offset 32
    float DiffuseIntensity;        // offset 44
    float Atten_Constant;          // offset 48
    float Atten_Linear;            // offset 52
    float Atten_Exp;               // offset 56
    float Cutoff;                  // offset 60
};



layout(push_constant) uniform Constants {
    uint BaseTextureIndex; 
} push;

layout(constant_id = 0) const uint LIGHTING_MODE = LIGHTING_MODE_FULL;

layout(location = 0) in vec2 TexCoord;
layout(location = 1) flat in uint MaterialIndex;
layout(location = 2) in vec3 WorldPos;
layout(location = 3) in vec3 Normal;
layout(location = 4) in vec4 Color;

layout(location = 0) out vec4 OutColor;

layout(set = 0, binding = 0) uniform sampler2D Textures[];

layout(set = 1, binding = 4) readonly uniform UniformBuffer { 
    ivec4 Counts;              // .x = NumLights, .y = NumPointLights, .z = NumSpotLights, .w = NumDirectionalLights
    vec4 CameraWorldPos;
    vec4 Material;            // .x = specular intensity
    LightSource LightSources[MAX_NUM_LIGHTS];
} ubo;

vec3 GeoNormal;

vec4 TextureBindless2D(uint MaterialIndex, vec2 uv) 
{
    uint Index = push.BaseTextureIndex + MaterialIndex;
    return texture(Textures[nonuniformEXT(Index)], uv);
}


void main() 
{
    GeoNormal = normalize(Normal);

    vec4 BaseColor = TextureBindless2D(MaterialIndex, TexCoord) * Color;

    vec3 LightColor = ubo.LightSources[0].Color;
    float AmbientIntensity = ubo.LightSources[0].AmbientIntensity;
    vec3 LightDirection = -ubo.LightSources[0].Direction;
    float DiffuseIntensity = ubo.LightSources[0].DiffuseIntensity;

    switch (LIGHTING_MODE) {
        case LIGHTING_MODE_UNLIT:
            OutColor = BaseColor;
            break;

        case LIGHTING_MODE_NORMALS:
            OutColor = vec4(GeoNormal * 0.5 + 0.5, 1.0);
            break;

        case LIGHTING_MODE_AMBIENT_ONLY:
            OutColor = vec4(BaseColor.rgb * LightColor * AmbientIntensity, BaseColor.a);
            break;

        case LIGHTING_MODE_FULL:
            // 1. Normalize Vectors
            vec3 N = GeoNormal;
    
            // We negate the Direction so L points TOWARD the light source for the dot product
            vec3 L = normalize(LightDirection); 

            // 2. Diffuse (Lambertian) Calculation
            float NdotL = max(dot(N, L), 0.0);
    
            // 4. Calculate Lighting Components
            // Ambient: Base Texture * Ambient Color * Ambient Intensity
            vec3 Ambient = BaseColor.rgb * LightColor * AmbientIntensity;

            // Diffuse: Base Texture * Light Color * Light Intensity * NdotL
            vec3 Diffuse = BaseColor.rgb * LightColor * DiffuseIntensity * NdotL;

            // 5. Final Output
            OutColor = vec4(Ambient + Diffuse, BaseColor.a);

            break;

        default:
            OutColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
}
