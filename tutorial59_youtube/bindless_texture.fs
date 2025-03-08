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

#extension GL_ARB_bindless_texture : require

uniform int gTextureIndex = 0;

layout(binding = 0, std430) readonly buffer ssbo3 {
    sampler2D textures[];
};

in vec2 TexCoord;

out vec4 FragColor;

void main() 
{   
    sampler2D tex = textures[gTextureIndex];
    
    FragColor = vec4(texture(tex, TexCoord).rgb, 1.0);
}