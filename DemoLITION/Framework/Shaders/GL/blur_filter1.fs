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


layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D gSampler;

in vec2 TexCoords;

uniform float PixOffset[10] = float[](0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0);
uniform float Weights[10];

void main()
{
    float dy = 1.0 / (textureSize(gSampler, 0)).y;

    vec4 sum = texture(gSampler, TexCoords) * Weights[0];

    for ( int i = 1; i < 10; i++ ) {
         sum += texture(gSampler, TexCoords + vec2(0.0,PixOffset[i]) * dy ) * Weights[i];
         sum += texture(gSampler, TexCoords - vec2(0.0,PixOffset[i]) * dy ) * Weights[i];
    }

    FragColor = sum;
}
