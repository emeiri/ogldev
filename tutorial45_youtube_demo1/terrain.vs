/*
    Copyright 2022 Etay Meiri

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
layout (location = 1) in vec2 InTex;

uniform float gTime;
uniform vec3 gCenter = vec3(1000.0, 0.0, 1000.0);

out VS_OUT {
    vec2 Tex;
} vs_out;


void main()
{
    gl_Position = vec4(Position, 1.0);

    float Distance = length(Position - gCenter);
    gl_Position.y = sin(Distance / 12.0 + gTime) * 8.0;

    vs_out.Tex = InTex;
}
