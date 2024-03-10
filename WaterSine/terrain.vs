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
layout (location = 2) in vec3 InNormal;

uniform mat4 gVP;
uniform float gMinHeight;
uniform float gMaxHeight;

out vec4 Color;
out vec2 Tex;
out vec3 WorldPos;
out vec3 Normal;

void main()
{
    gl_Position = gVP * vec4(Position, 1.0);

    float DeltaHeight = gMaxHeight - gMinHeight;

    float HeightRatio = (Position.y - gMinHeight) / DeltaHeight;

    float c = HeightRatio * 0.8 + 0.2;

    Color = vec4(c, c, c, 1.0);

    Tex = InTex;
    
    WorldPos = Position;
    
    Normal = InNormal;
}
