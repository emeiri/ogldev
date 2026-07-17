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

layout (binding = 1) uniform sampler2D gTexture0; // Meaning: Low Elevation (e.g. Sand)
layout (binding = 2) uniform sampler2D gTexture1; // Meaning: Flat Ground / Mid Elevation (e.g. Grass)
layout (binding = 3) uniform sampler2D gTexture2; // Meaning: Steep Slopes / High Elevation (e.g. Rock)

layout (location = 3) uniform float gMaxTerrainHeight = 50.0f;

const float gBeachHeight = gMaxTerrainHeight * 0.25;     
const float gMountainHeight = gMaxTerrainHeight * 0.7; 
uniform vec3 u_SunDirection = normalize(vec3(0.5, 1.0, 0.3)); 

void main()
{
    vec2 DetailUV = TexCoords * 8; 

    vec3 N = normalize(Normal);
    float SlopeFactor = smoothstep(0.65, 0.85, N.y); 
     float grassMask = smoothstep(gBeachHeight - 2.0, gBeachHeight + 2.0, WorldPos.y);

    float snowMask = smoothstep(gMountainHeight - 5.0, gMountainHeight + 5.0, WorldPos.y);

    vec4 sand   = texture(gTexture0, DetailUV);
    vec4 grass   = texture(gTexture1, DetailUV);
    vec4 snow  = texture(gTexture2, DetailUV);


   vec4 sandAndGrass = mix(sand, grass, grassMask);
    vec4 FinalColor = mix(sandAndGrass, snow, snowMask);
   // FragColor = vec4(Normal * 0.5 + 0.5, 1.0);//gColor;
   FragColor = FinalColor;
   //FragColor = vec4(vec3(WorldPos.y / 100.0), 1.0);
   
// Proceed with your FinalColor mix
//vec4 FinalColor = mix(lowAndMid, LayerHigh, highWeight);
//FragColor = vec4(WorldPos.y / 50.0);
}
