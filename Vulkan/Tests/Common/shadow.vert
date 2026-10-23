/*

        Copyright 2026 Etay Meiri

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

#define MAX_NUM_LIGHTS 16

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
const int INDEX_SIZE_IN_BYTES = 4;

layout(push_constant) uniform Constants {
    uint LightIndex; 
} push;


layout (std430, set = 1, binding = 0) readonly buffer Vertices { VertexData v[]; } in_Vertices;

layout (std430, set = 1, binding = 1) readonly buffer Indices { int i[]; } in_Indices;

struct UniformData {
    mat4 WVP; 
    mat4 WorldTransform;
    mat4 NormalTransform;
};

// Changed from uniform buffer to SSBO to support unlimited meshes
// SSBOs don't have the 64KB size limit of uniform buffers
layout (std430, set = 1, binding = 2) readonly buffer TransformBuffer { 
    UniformData Data[];
} ubo;

struct MetaData { 
    uint MaterialIndex; 
    uint IndexOffset; 
    uint IndexCount; 
    uint VertexOffset; 
    vec4 Color;
};

layout(std430, set = 1, binding = 3) readonly buffer MetaSSBO { 
    MetaData metas[]; 
} MetaBuf;


layout(set = 1, binding = 6) uniform LightMats {
    ivec4 Counts;              // .x = NumLights, .y = NumPointLights, .z = NumSpotLights, .w = NumDirectionalLights
    mat4 LightVP[MAX_NUM_LIGHTS];
} LightMatrices;

void main() 
{    
    uint DrawId = uint(gl_BaseInstance);

    MetaData md = MetaBuf.metas[DrawId];

    uint BaseIndex = md.IndexOffset / INDEX_SIZE_IN_BYTES;
    int Index = in_Indices.i[BaseIndex + gl_VertexIndex];

    uint BaseVertex = md.VertexOffset / (NUM_FLOATS_IN_VERTEX_DATA * FLOAT_SIZE_IN_BYTES);
    VertexData vtx = in_Vertices.v[BaseVertex + Index];

    vec4 Pos4d = vec4(vtx.pos_x, vtx.pos_y, vtx.pos_z, 1.0);

    mat4 WorldTransform = ubo.Data[DrawId].WorldTransform;
    mat4 WVTransform = LightMatrices.LightVP[push.LightIndex];

    gl_Position = WVTransform * WorldTransform * Pos4d;    
}
