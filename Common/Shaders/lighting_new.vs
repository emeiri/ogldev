#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec3 Normal;

uniform mat4 gWVP;
uniform mat4 gLightWVP; // required only for shadow mapping (spot/directional light)
uniform mat4 gWorld;
uniform vec4 gClipPlane;

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 LocalPos0;
out vec3 WorldPos0;
out vec4 LightSpacePos0; // required only for shadow mapping (spot/directional light)
out vec3 EdgeDistance0; // to match lighting_new_to_vs.gs

void main()
{
    vec4 Pos4 = vec4(Position, 1.0);
    gl_Position = gWVP * Pos4;
    TexCoord0 = TexCoord;
    Normal0 = Normal;
    LocalPos0 = Position;
    WorldPos0 = (gWorld * Pos4).xyz;
    LightSpacePos0 = gLightWVP * Pos4; // required only for shadow mapping (spot/directional light)
    EdgeDistance0 = vec3(-1.0, -1.0, -1.0);   // used only by wireframe_on_mesh.gs

    gl_ClipDistance[0] = dot(vec4(Position, 1.0), gClipPlane);
}
