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

#version 450 core

// Fullscreen triangle trick
const vec2 verts[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0,  3.0),
    vec2( 3.0, -1.0)
);

out vec2 TexCoords;

void main()
{
    vec2 Pos = verts[gl_VertexID];
    gl_Position = vec4(Pos, 0.0, 1.0);

    // Map from [-1,1] to [0,1]
    TexCoords = Pos * 0.5 + 0.5;
}
