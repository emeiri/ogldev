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

struct VertexData
{
	float x, y, z;
	float u, v;
};

layout (binding = 0) readonly buffer Vertices { VertexData data[]; } in_Vertices;

layout (binding = 2) readonly uniform UniformBuffer { mat4 WVP; } ubo;

layout(location = 0) out vec2 texCoord;

void main() 
{
	VertexData vtx = in_Vertices.data[gl_VertexIndex];

	vec3 pos = vec3(vtx.x, vtx.y, vtx.z);

	gl_Position = ubo.WVP * vec4(pos, 1.0);

	texCoord = vec2(vtx.u, vtx.v);
}

