struct VSInput 
{
    vec3  Position;                                             
    vec2  TexCoord;                                             
    vec3  Normal;    
    ivec4 BoneIDs;
    vec4  Weights;
};

const int MAX_BONES = 100;

uniform mat4 gWVP;
uniform mat4 gWorld;
uniform mat4 gBones[MAX_BONES];

shader VSmain(in VSInput VSin:0)
{       
    mat4 BoneTransform = gBones[VSin.BoneIDs[0]] * VSin.Weights[0];
    BoneTransform     += gBones[VSin.BoneIDs[1]] * VSin.Weights[1];
    BoneTransform     += gBones[VSin.BoneIDs[2]] * VSin.Weights[2];
    BoneTransform     += gBones[VSin.BoneIDs[3]] * VSin.Weights[3];

    vec4 PosL   = BoneTransform * vec4(VSin.Position, 1.0);
    gl_Position = gWVP * PosL;
}

                                                                                            
shader FSmain()
{                                    
}

program Depth
{
    vs(330)=VSmain();
    fs(330)=FSmain();
};