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


#version 460 core

//
// Non PVP input attributes
//
layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 inTexCoord0;
layout (location = 2) in vec2 inTexCoord1;
layout (location = 3) in vec3 Normal;
layout (location = 4) in vec3 Tangent;
layout (location = 5) in vec3 Bitangent;
layout (location = 6) in vec4 Color;



// 
// PVP input attributes
//
struct Vertex {
    float Position[3];
    float inTexCoord0[2];
    float inTexCoord1[2];
    float Normal[3];
    float Tangent[3];
    float Bitangent[3];
    float Color[4];
};


layout(std430, binding = 0) restrict readonly buffer Vertices {
    Vertex in_Vertices[];
};


struct PerObjectData {
    mat4 WorldMatrix;
    mat4 NormalMatrix;
    ivec4 MaterialIndex;
};


layout(std430, binding = 1) restrict readonly buffer PerObjectSSBO {
    PerObjectData o[];
};

out vec3 Normal0;
out vec2 TexCoord0;

uniform mat4 gWVP;
uniform mat4 gVP;
uniform mat3 gNormalMatrix;
uniform bool gIsPVP = false;
uniform bool gIsIndirectRender = false;

vec3 GetPosition(int i)
{
    return vec3(in_Vertices[i].Position[0], 
                in_Vertices[i].Position[1], 
                in_Vertices[i].Position[2]);
}


vec3 GetNormal(int i)
{
    return vec3(in_Vertices[i].Normal[0], 
                in_Vertices[i].Normal[1], 
                in_Vertices[i].Normal[2]);
}


vec2 GetTexCoord0(int i)
{
    return vec2(in_Vertices[i].inTexCoord0[0], 
                in_Vertices[i].inTexCoord0[1]);
}


void main()
{
    vec3 Position_;
    vec3 Normal_;
    vec2 TexCoord0_;
    
    if (gIsPVP) {
        Position_ = GetPosition(gl_VertexID);        
        Normal_ = GetNormal(gl_VertexID);
        TexCoord0_ = GetTexCoord0(gl_VertexID);
    } else {
        Position_ = Position;
        Normal_ = Normal;
        TexCoord0_ = TexCoord0;
    }

    vec4 Pos4 = vec4(Position_, 1.0);

    if (gIsIndirectRender) {
        gl_Position = gVP * o[gl_DrawID].WorldMatrix * Pos4;
        Normal0 = (o[gl_DrawID].NormalMatrix * vec4(Normal_, 0.0)).xyz;
    } else {
        gl_Position = gWVP * Pos4;
        Normal0 = gNormalMatrix * Normal_;
    }

    TexCoord0 = TexCoord0_;
}
