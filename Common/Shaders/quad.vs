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

#version 400

const vec3 Pos[4] = vec3[4](
    vec3(-1.0, -1.0, 0.5),      // bottom left
    vec3( 1.0, -1.0, 0.5),      // bottom right
    vec3( 1.0,  1.0, 0.5),      // top right
    vec3(-1.0,  1.0, 0.5)       // top left
);


const vec2 TexCoords[4] = vec2[4](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
);

const int Indices[6] = int[6](0, 2, 1, 2, 0, 3);

out vec2 TexCoord;

void main()
{
    int Index = Indices[gl_VertexID];
    vec3 vPos3 = Pos[Index];

    gl_Position = vec4(vPos3, 1.0);
    TexCoord = TexCoords[Index];
}

