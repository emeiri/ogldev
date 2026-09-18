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

#version 460

// Fullscreen triangle trick
const vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0), // 0: Top-Left
    vec2(-1.0,  3.0), // 1: Far Bottom-Left extension
    vec2( 3.0, -1.0)  // 2: Far Top-Right extension
);

const vec2 uvs[3] = vec2[](
    vec2(0.0, 1.0), // 0: Top-Left UV
    vec2(0.0, -1.0), // 1: Far Bottom-Left UV
    vec2(2.0, 1.0)  // 2: Far Top-Right UV
);

layout(location = 0) out vec2 TexCoords;

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    TexCoords = uvs[gl_VertexIndex];
}
