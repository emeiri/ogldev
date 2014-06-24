#version 330                                                                        
                                                                                    
layout (location = 0) in vec3 Position;                                             
layout (location = 1) in vec2 TexCoord;                                             
layout (location = 2) in vec3 Normal;                                               
layout (location = 3) in ivec4 BoneIDs;
layout (location = 4) in vec4 Weights;

out vec2 TexCoord0;
out vec3 Normal0;                                                                   
out vec3 WorldPos0;                                                                 
out vec4 ClipSpacePos0;
out vec4 PrevClipSpacePos0;                                                                


const int MAX_BONES = 100;

uniform mat4 gWVP;
uniform mat4 gWorld;
uniform mat4 gBones[MAX_BONES];
uniform mat4 gPrevBones[MAX_BONES];

void main()
{       
    mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
    BoneTransform     += gBones[BoneIDs[1]] * Weights[1];
    BoneTransform     += gBones[BoneIDs[2]] * Weights[2];
    BoneTransform     += gBones[BoneIDs[3]] * Weights[3];

    vec4 PosL         = BoneTransform * vec4(Position, 1.0);
    vec4 ClipSpacePos = gWVP * PosL;
    gl_Position       = ClipSpacePos;
    TexCoord0         = TexCoord;
    vec4 NormalL      = BoneTransform * vec4(Normal, 0.0);
    Normal0           = (gWorld * NormalL).xyz;
    WorldPos0         = (gWorld * PosL).xyz;                                

    mat4 PrevBoneTransform = gPrevBones[BoneIDs[0]] * Weights[0];
    PrevBoneTransform += gPrevBones[BoneIDs[1]] * Weights[1];
    PrevBoneTransform += gPrevBones[BoneIDs[2]] * Weights[2];
    PrevBoneTransform += gPrevBones[BoneIDs[3]] * Weights[3];

    ClipSpacePos0 = ClipSpacePos;
    vec4 PrevPosL = PrevBoneTransform * vec4(Position, 1.0);
    PrevClipSpacePos0 = gWVP * PrevPosL;                          
}
