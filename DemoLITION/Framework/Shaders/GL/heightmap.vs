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
out vec3 FragPos;

layout (location = 0) uniform mat4 gWVP;

layout (binding = 0) uniform sampler2D heightMap; 
layout (location = 3) uniform float maxTerrainHeight = 350.0f;

void main()
{
    TexCoords = aTexCoords;

    // Fetch from your 32-bit R32F floating-point texture
    float heightSample = texture(heightMap, aTexCoords).r;

    // Displace the vertex position vertically in metric space
    vec3 displacedPosition = aPos;
    displacedPosition.y = heightSample * maxTerrainHeight;

    //FragPos = vec3(model * vec4(displacedPosition, 1.0));
    gl_Position = gWVP * vec4(aPos, 1.0);
}
