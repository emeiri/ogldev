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

vec2 pos[3] = vec2[3]( vec2(-0.7, 0.7), vec2(0.7, 0.7), vec2(0.0, -0.7) ); 

void main() 
{
	gl_Position = vec4( pos[gl_VertexIndex], 0.0, 1.0 );
}

