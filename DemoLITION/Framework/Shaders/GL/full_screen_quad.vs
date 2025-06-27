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

#version 330

const vec3 Pos[6] = vec3[6](
    vec3(-1.0, -1.0, 0.5),      // bottom left
    vec3(-1.0, 1.0,  0.5),      // top left
    vec3( 1.0, 1.0,  0.5),      // top right

    vec3(-1.0, -1.0, 0.5),      // bottom left
    vec3( 1.0, 1.0,  0.5),      // top right
    vec3( 1.0, -1.0, 0.5)       // bottom right      
);

out vec2 TexCoords;


void main()
{
    vec4 Pos4 = vec4(Pos[gl_VertexID], 1.0);

    gl_Position = Pos4;

    TexCoords = vec2((Pos4.x + 1.0) / 2.0, (Pos4.y + 1.0) / 2.0);
}