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
   // 1. Normalize Vectors
    vec3 N = normalize(Normal);
    
    // We negate the Direction so L points TOWARD the light source for the dot product
    vec3 L = normalize(-ubo.LightDirection.xyz); 

    // 2. Diffuse (Lambertian) Calculation
    float NdotL = max(dot(N, L), 0.0);

    // 3. Sample Base Texture
    vec4 BaseColor = TextureBindless2D(MaterialIndex, TexCoord);

    // 4. Calculate Lighting Components
    // Ambient: Base Texture * Ambient Color * Ambient Intensity
    vec3 Ambient = BaseColor.rgb * ubo.AmbientLight.rgb * ubo.AmbientLight.w;

    // Diffuse: Base Texture * Light Color * Light Intensity * NdotL
    vec3 Diffuse = BaseColor.rgb * ubo.LightColor.rgb * ubo.LightColor.w * NdotL;

    // 5. Final Output
    OutColor = vec4(Ambient + Diffuse, BaseColor.a);
}
