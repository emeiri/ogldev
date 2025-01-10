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

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 out_Color;

layout(binding = 2) uniform sampler2D texSampler;

void main() {
//  out_Color = vec4( 0.0, 0.4, 1.0, 1.0 ); 
  out_Color = texture(texSampler, uv);
}
