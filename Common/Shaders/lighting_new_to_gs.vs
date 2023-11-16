// 
// Same as lighting_new.vs except that the names of the output variables are different
// so that a GS can be dropped in and output the original names for the existing fs.
//

#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec3 Normal;

uniform mat4 gWVP;
uniform mat4 gLightWVP; // required only for shadow mapping (spot/directional light)
uniform mat4 gWorld;
uniform vec4 gClipPlane;

out vec2 VTexCoord;
out vec3 VNormal;
out vec3 VLocalPos;
out vec3 VWorldPos;
out vec4 VLightSpacePos; // required only for shadow mapping (spot/directional light)

void main()
{
    vec4 Pos4 = vec4(Position, 1.0);
    gl_Position = gWVP * Pos4;
    VTexCoord = TexCoord;
    VNormal = Normal;
    VLocalPos = Position;
    VWorldPos = (gWorld * Pos4).xyz;
    VLightSpacePos = gLightWVP * vec4(Position, 1.0); // required only for shadow mapping (spot/directional light)

    gl_ClipDistance[0] = dot(vec4(Position, 1.0), gClipPlane);
}
