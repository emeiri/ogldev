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

uniform mat4 gWVP;
uniform mat4 gLightWVP; // required only for shadow mapping (spot/directional light)
uniform mat4 gWorld;
uniform vec4 gClipPlane;

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 LocalPos0;
out vec3 WorldPos0;
out vec4 LightSpacePos0; // required only for shadow mapping (spot/directional light)
noperspective out vec3 EdgeDistance0; // to match lighting_new_to_vs.gs

void main()
{
    vec4 Pos4 = vec4(Position, 1.0);
    gl_Position = gWVP * Pos4;
    TexCoord0 = TexCoord;
    Normal0 = Normal;
    LocalPos0 = Position;
    WorldPos0 = (gWorld * Pos4).xyz;
    LightSpacePos0 = gLightWVP * Pos4; // required only for shadow mapping (spot/directional light)
    EdgeDistance0 = vec3(-1.0, -1.0, -1.0);   // used only by wireframe_on_mesh.gs

    gl_ClipDistance[0] = dot(vec4(Position, 1.0), gClipPlane);
}
