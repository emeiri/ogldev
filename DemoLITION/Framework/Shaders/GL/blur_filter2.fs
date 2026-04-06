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

uniform float Weights[10];

void main()
{
    float dx = 1.0 / (textureSize(gSampler, 0)).x;

    vec4 sum = texture(gSampler, TexCoords) * Weights[0];

    for( int i = 1; i < 10; i++ ) {
	    vec2 Offset = vec2(i * dx, 0.0f);
        float w = Weights[i];
        sum += texture(gSampler, TexCoords + Offset) * w;  // right
        sum += texture(gSampler, TexCoords - Offset) * w;  // left
    }

    FragColor = sum;
}
