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

layout (location = 0) in vec3 aPos;       
layout (location = 1) in vec2 aTexCoords; 

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

layout (location = 0) uniform mat4 gWVP;

layout (binding = 0) uniform sampler2D gHeightMap; 
layout (location = 3) uniform float gMaxTerrainHeight = 150.0f;
layout (location = 4) uniform float gHorizontalScale = 3.0f;


void main()
{
    TexCoords = aTexCoords;

    // Fetch from your 32-bit R32F floating-point texture
    float HeightSample = texture(gHeightMap, aTexCoords).r;

    // Displace the vertex position vertically in metric space
    vec3 DisplacedPos = aPos;
    DisplacedPos.x *= gHorizontalScale;
    DisplacedPos.y = HeightSample * gMaxTerrainHeight;
    DisplacedPos.z *= gHorizontalScale;

    ivec2 TexSize = textureSize(gHeightMap, 0);
    float TexelSize = 1.0 / float(TexSize.x);
    float hL = textureOffset(gHeightMap, aTexCoords, ivec2(-1,  0)).r;
    float hR = textureOffset(gHeightMap, aTexCoords, ivec2( 1,  0)).r;
    float hD = textureOffset(gHeightMap, aTexCoords, ivec2( 0, -1)).r;
    float hU = textureOffset(gHeightMap, aTexCoords, ivec2( 0,  1)).r;
    
    Normal = normalize(normalize(vec3((hL - hR) * gMaxTerrainHeight, 2.0, (hD - hU) * gMaxTerrainHeight)));

    //Normal = normalize(vec3((hL - hR) * 100.0, 1.0, (hD - hU) * 100.0));

    WorldPos = DisplacedPos;
    gl_Position = gWVP * vec4(DisplacedPos, 1.0);
}
