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
layout (location = 1) in vec2 TexCoords;

uniform mat4 gWVP = mat4(1.0);

out vec2 TexCoords0;

void main()
{
    gl_Position = gWVP * vec4(Position, 1.0);
    TexCoords0 = TexCoords;
}
