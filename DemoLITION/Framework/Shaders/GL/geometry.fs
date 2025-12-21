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

#version 420

layout(location = 0) out vec4 AlbedoColor;
layout(location = 1) out vec4 Normal;

layout(binding = 0) uniform sampler2D gAlbedoSampler;

in vec3 Normal0;
in vec2 TexCoord0;

void main()
{
    AlbedoColor = texture(gAlbedoSampler, TexCoord0.xy);
    Normal = vec4(normalize(Normal0), 0.0);
}
