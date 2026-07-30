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

layout (location = 0) in vec2 aPos;       
layout (location = 1) in vec2 aTexCoords; 

out vec3 WorldPos;
out vec2 TexCoords;
out vec3 Normal;
out float OrigHeight;

layout (binding = 0) uniform sampler2D gHeightMap; 

uniform mat4 gWVP;
uniform float gMaxTerrainHeight = 100.0f;
uniform float gHorizontalScale = 4.0f;


void main()
{
    float HeightSample = texture(gHeightMap, aTexCoords).r;

    // Displace the vertex position vertically in metric space
    vec3 DisplacedPos;
    DisplacedPos.x = aPos.x * gHorizontalScale;
    DisplacedPos.y = HeightSample * gMaxTerrainHeight;
    DisplacedPos.z = aPos.y * gHorizontalScale;

    float hL = textureOffset(gHeightMap, aTexCoords, ivec2(-1, 0)).r * gMaxTerrainHeight;
    float hR = textureOffset(gHeightMap, aTexCoords, ivec2( 1, 0)).r * gMaxTerrainHeight;
    float hD = textureOffset(gHeightMap, aTexCoords, ivec2( 0, -1)).r * gMaxTerrainHeight;
    float hU = textureOffset(gHeightMap, aTexCoords, ivec2( 0, 1)).r * gMaxTerrainHeight;
    
    float stepDist = 2.0 * gHorizontalScale;
    Normal = normalize(vec3(hL - hR, stepDist, hD - hU));

    WorldPos = DisplacedPos;
    TexCoords = aTexCoords;
    OrigHeight = HeightSample;
    gl_Position = gWVP * vec4(DisplacedPos, 1.0);
}
