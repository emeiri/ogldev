#version 430

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VTexCoord;

out vec3 WorldPosition;
out vec3 ViewPosition;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 WorldMatrix;
uniform mat4 ViewMatrix;
uniform mat4 NormalMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
    Normal = normalize( mat3(NormalMatrix) * VertexNormal);
    WorldPosition = vec3( WorldMatrix * vec4(VertexPosition,1.0) );
    ViewPosition = vec3( ViewMatrix * vec4(WorldPosition, 1.0) );
    TexCoord = VTexCoord;

    gl_Position = ProjectionMatrix * vec4(ViewPosition,1.0);
}
