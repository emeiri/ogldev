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

// Any change here must be reflected in NUM_FLOATS_IN_VERTEX_DATA!
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

const int NUM_FLOATS_IN_VERTEX_DATA = 20;
const int FLOAT_SIZE_IN_BYTES = 4;

layout (std430, set = 0, binding = 0) readonly buffer Vertices { VertexData v[]; } in_Vertices;

layout (set = 0, binding = 1) readonly buffer Indices { int i[]; } in_Indices;

layout (set = 0, binding = 3) readonly uniform UniformBuffer { mat4 WVP[1024]; } ubo;

struct MetaData { 
    uint MaterialIndex; 
    uint IndexOffset; 
    uint IndexCount; 
    uint VertexOffset; 
};

layout(std430, set = 0, binding = 4) readonly buffer MetaSSBO { MetaData metas[]; } MetaBuf;

layout(location = 0) out vec2 texCoord;
layout(location = 1) flat out uint MaterialIndex;

const int INDEX_SIZE_IN_BYTES = 4;

void main() 
{    
    uint DrawId = uint(gl_BaseInstance);

    MetaData md = MetaBuf.metas[DrawId];

    MaterialIndex = md.MaterialIndex;

    uint BaseIndex = md.IndexOffset / INDEX_SIZE_IN_BYTES;
    int Index = in_Indices.i[BaseIndex + gl_VertexIndex];

    uint BaseVertex = md.VertexOffset / (NUM_FLOATS_IN_VERTEX_DATA * FLOAT_SIZE_IN_BYTES);
    VertexData vtx = in_Vertices.v[BaseVertex + Index];

    vec3 pos = vec3(vtx.pos_x, vtx.pos_y, vtx.pos_z);

    gl_Position = ubo.WVP[DrawId] * vec4(pos, 1.0);
    
    texCoord = vec2(vtx.u0, vtx.v0);
}

