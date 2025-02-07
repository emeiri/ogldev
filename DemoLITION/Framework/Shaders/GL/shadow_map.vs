/*

        Copyright 2025 Etay Meiri

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

layout (location = 0) in vec3 Position;

uniform mat4 gWVP;
uniform mat4 gVP;
uniform bool gIsIndirectRender = false;

struct PerObjectData {
    mat4 WorldMatrix;
    mat4 NormalMatrix;
    ivec4 MaterialIndex;
};


layout(std430, binding = 1) restrict readonly buffer PerObjectSSBO {
    PerObjectData o[];
};


void main()
{
    vec4 Pos4 = vec4(Position, 1.0);

    if (gIsIndirectRender) {
        gl_Position = gVP * o[gl_DrawID].WorldMatrix * Pos4;
    } else {
        gl_Position = gWVP * Pos4;
    }
}
