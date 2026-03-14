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

layout(constant_id = 0) const uint LIGHTING_MODE = LIGHTING_MODE_FULL;

layout(location = 0) in vec2 TexCoord;
layout(location = 1) flat in uint MaterialIndex;
layout(location = 2) in vec3 Normal;

layout(location = 0) out vec4 OutColor;

layout(set = 0, binding = 2) uniform sampler2D Textures[];

layout(set = 0, binding = 5) readonly uniform UniformBuffer { 
    vec4 AmbientLight;      // .rgb = color, .w = intensity
    vec4 LightDirection;    // .xyz = direction
    vec4 LightColor;        // .rgb = color, .w = intensity
} ubo;


vec4 TextureBindless2D(uint MaterialIndex, vec2 uv) 
{
     return texture(Textures[nonuniformEXT(MaterialIndex)], uv);
}


void main() 
{
    vec4 BaseColor = TextureBindless2D(MaterialIndex, TexCoord);

    switch (LIGHTING_MODE) {
        case LIGHTING_MODE_UNLIT:
            OutColor = BaseColor;
            break;

        case LIGHTING_MODE_NORMALS:
            OutColor = vec4(normalize(Normal) * 0.5 + 0.5, 1.0);
            break;

        case LIGHTING_MODE_AMBIENT_ONLY:
            OutColor = vec4(BaseColor.rgb * ubo.AmbientLight.rgb * ubo.AmbientLight.w, BaseColor.a);
            break;

        case LIGHTING_MODE_FULL:
            // 1. Normalize Vectors
            vec3 N = normalize(Normal);
    
            // We negate the Direction so L points TOWARD the light source for the dot product
            vec3 L = normalize(-ubo.LightDirection.xyz); 

            // 2. Diffuse (Lambertian) Calculation
            float NdotL = max(dot(N, L), 0.0);
    
            // 4. Calculate Lighting Components
            // Ambient: Base Texture * Ambient Color * Ambient Intensity
            vec3 Ambient = BaseColor.rgb * ubo.AmbientLight.rgb * ubo.AmbientLight.w;

            // Diffuse: Base Texture * Light Color * Light Intensity * NdotL
            vec3 Diffuse = BaseColor.rgb * ubo.LightColor.rgb * ubo.LightColor.w * NdotL;

            // 5. Final Output
            OutColor = vec4(Ambient + Diffuse, BaseColor.a);

            break;

        default:
            OutColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
}
