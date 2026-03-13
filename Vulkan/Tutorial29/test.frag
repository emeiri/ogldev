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


#version 460

#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 texCoord;
layout(location = 1) flat in uint MaterialIndex;

layout(location = 0) out vec4 out_Color;

layout(set = 0, binding = 2) uniform sampler2D Textures[];

vec4 TextureBindless2D(uint MaterialIndex, vec2 uv) 
{
     return texture(Textures[nonuniformEXT(MaterialIndex)], uv);
}


void main() 
{
    out_Color = TextureBindless2D(MaterialIndex, texCoord);
}
