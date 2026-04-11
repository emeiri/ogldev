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

uniform float gLuminanceThreshold = 0.3;

float luminance(vec3 color)
{
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}


void main()
{
   vec4 val = texture(gSampler, TexCoords);
   float lum = luminance(val.rgb);

   // Only output the "excess" above the threshold (soft-knee)
   if (lum > gLuminanceThreshold) {
       FragColor = vec4(val.rgb - gLuminanceThreshold, val.a);
   } else {
       FragColor = vec4(0.0);
   }
}
