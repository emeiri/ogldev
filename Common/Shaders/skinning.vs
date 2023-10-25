#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in ivec4 BoneIDs;
layout (location = 4) in vec4 Weights;

const int MAX_BONES = 200;

uniform mat4 gWVP;
uniform mat4 gBones[MAX_BONES];
uniform mat4 gWorld;
uniform mat4 gLightWVP; // required only for shadow mapping (spot/directional light)
uniform vec4 gClipPlane;

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 LocalPos0;
out vec3 WorldPos0;
out vec4 LightSpacePos; // required only for shadow mapping (spot/directional light)


void main()
{
    mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
    BoneTransform     += gBones[BoneIDs[1]] * Weights[1];
    BoneTransform     += gBones[BoneIDs[2]] * Weights[2];
    BoneTransform     += gBones[BoneIDs[3]] * Weights[3];

    vec4 PosL = BoneTransform * vec4(Position, 1.0);
    gl_Position = gWVP * PosL;
    TexCoord0 = TexCoord;
    Normal0 = Normal;
    LocalPos0 = PosL.xyz;
    WorldPos0 = (gWorld * PosL).xyz;
    LightSpacePos = gLightWVP * vec4(Position, 1.0); // required only for shadow mapping (spot/directional light)
    gl_ClipDistance[0] = dot(vec4(Position, 1.0), gClipPlane);
}
