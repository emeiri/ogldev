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

layout (location=0) out vec3 Dir;

layout (binding = 2) readonly uniform UniformBuffer { mat4 VP; } ubo;

const vec3 pos[8] = vec3[8](
	vec3(-1.0,-1.0, 1.0),
	vec3( 1.0,-1.0, 1.0),
	vec3( 1.0, 1.0, 1.0),
	vec3(-1.0, 1.0, 1.0),

	vec3(-1.0,-1.0,-1.0),
	vec3( 1.0,-1.0,-1.0),
	vec3( 1.0, 1.0,-1.0),
	vec3(-1.0, 1.0,-1.0)
);

const int indices[36] = int[36](
	1, 0, 2, 3, 2, 0,	// front
	5, 1, 6, 2, 6, 1,	// right 
	6, 7, 5, 4, 5, 7,	// back
	0, 4, 3, 7, 3, 4,	// left
	5, 4, 1, 0, 1, 4,	// bottom
	2, 3, 6, 7, 6, 3	// top
);

void main() 
{
	int idx = indices[gl_VertexIndex];
	vec4 Pos = vec4(pos[idx], 1.0);
	vec4 WVP_Pos = ubo.VP * Pos;
	gl_Position = WVP_Pos.xyww;
	Dir = pos[idx].xyz;
}
