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
    float pos_x, pos_y, pos_z;
    float u0, v0;
    float u1, v1;
    float normal_x, normal_y, normal_z;
    float tangent_x, tangent_y, tangent_z;
    float bitangent_x, bitangent_y, bitangent_z;
    float color_r, color_g, color_b, color_a;
};

layout (std430, binding = 0) readonly buffer Vertices { VertexData v[]; } in_Vertices;

layout (binding = 1) readonly buffer Indices { int i[]; } in_Indices;

layout (binding = 2) readonly uniform UniformBuffer { mat4 WVP; } ubo;

layout(location = 0) out vec2 texCoord;

void main() 
{
    int Index = in_Indices.i[gl_VertexIndex];

    VertexData vtx = in_Vertices.v[Index];

    vec3 pos = vec3(vtx.pos_x, vtx.pos_y, vtx.pos_z);

    gl_Position = ubo.WVP * vec4(pos, 1.0);
    
    texCoord = vec2(vtx.u0, vtx.v0);
}

