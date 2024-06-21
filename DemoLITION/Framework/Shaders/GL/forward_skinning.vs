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


#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 Bitangent;
layout (location = 5) in ivec4 BoneIDs;
layout (location = 6) in vec4 Weights;

const int MAX_BONES = 200;

uniform mat4 gWVP;
uniform mat4 gLightWVP;
uniform mat4 gWorld;
uniform mat3 gNormalMatrix;
uniform mat4 gBones[MAX_BONES];

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 WorldPos0;
out vec4 LightSpacePos0;
out vec3 Tangent0;
out vec3 Bitangent0;

void main()
{
    mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
    BoneTransform     += gBones[BoneIDs[1]] * Weights[1];
    BoneTransform     += gBones[BoneIDs[2]] * Weights[2];
    BoneTransform     += gBones[BoneIDs[3]] * Weights[3];

    vec4 Pos4 = vec4(Position, 1.0);
    vec4 PosL = BoneTransform * Pos4;
    gl_Position = gWVP * PosL;
    TexCoord0 = TexCoord;
    Normal0 = gNormalMatrix * Normal;
    Tangent0 = gNormalMatrix * Tangent;
    Bitangent0 = gNormalMatrix * Bitangent;
    WorldPos0 = (gWorld * PosL).xyz;
    LightSpacePos0 = gLightWVP * PosL;
}
