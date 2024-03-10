#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;

uniform mat4 gVP;
uniform float gHeight = 0.0f;
uniform vec3 gCameraPos;

out vec2 oTex;
out vec4 ClipSpaceCoords;
out vec3 oVertexToCamera;

const float Tiling = 6.0;

void main()
{
    vec3 NewPosition = (Position + vec3(0.0, gHeight, 0.0));
    ClipSpaceCoords = gVP * vec4(NewPosition, 1.0);
    gl_Position = ClipSpaceCoords;
    oTex = TexCoord * Tiling;
    oVertexToCamera = gCameraPos - NewPosition;
}
