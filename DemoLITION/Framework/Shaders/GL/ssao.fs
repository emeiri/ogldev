/**/
#version 460 core

layout(location = 0) in vec2 TexCoords;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texDepth;
layout(binding = 1) uniform sampler2D texRotation;
layout(binding = 2) uniform sampler2D NormalMap;

const vec3 offsets[64] = vec3[64](
    vec3(0.049771, -0.044709, 0.049963),
    vec3(0.014575, 0.016531, 0.002239),
    vec3(-0.040648, -0.019375, 0.031934),
    vec3(0.013778, -0.091582, 0.040924),
    vec3(0.055989, 0.059792, 0.057659),
    vec3(0.092266, 0.044279, 0.015451),
    vec3(-0.002039, -0.054402, 0.066735),
    vec3(-0.000331, -0.000187, 0.000369),
    vec3(0.050044, -0.046650, 0.025385),
    vec3(0.038128, 0.031402, 0.032868),
    vec3(-0.031883, 0.020459, 0.022515),
    vec3(0.055702, -0.036974, 0.054492),
    vec3(0.057372, -0.022540, 0.075542),
    vec3(-0.016090, -0.003768, 0.055473),
    vec3(-0.025033, -0.024829, 0.024951),
    vec3(-0.033688, 0.021391, 0.025402),
    vec3(-0.017530, 0.014386, 0.005348),
    vec3(0.073359, 0.112052, 0.011014),
    vec3(-0.044056, -0.090284, 0.083683),
    vec3(-0.083277, -0.001683, 0.084987),
    vec3(-0.010406, -0.032867, 0.019273),
    vec3(0.003211, -0.004882, 0.004164),
    vec3(-0.007383, -0.065835, 0.067398),
    vec3(0.094141, -0.007998, 0.143350),
    vec3(0.076833, 0.126968, 0.106999),
    vec3(0.000393, 0.000450, 0.000302),
    vec3(-0.104793, 0.065445, 0.101737),
    vec3(-0.004452, -0.119638, 0.161901),
    vec3(-0.074553, 0.034449, 0.224138),
    vec3(-0.002758, 0.003078, 0.002923),
    vec3(-0.108512, 0.142337, 0.166435),
    vec3(0.046882, 0.103636, 0.059576),
    vec3(0.134569, -0.022512, 0.130514),
    vec3(-0.164490, -0.155644, 0.124540),
    vec3(-0.187666, -0.208834, 0.057770),
    vec3(-0.043722, 0.086925, 0.074797),
    vec3(-0.002564, -0.002001, 0.004070),
    vec3(-0.096696, -0.182259, 0.299487),
    vec3(-0.225767, 0.316061, 0.089156),
    vec3(-0.027505, 0.287187, 0.317177),
    vec3(0.207216, -0.270839, 0.110132),
    vec3(0.054902, 0.104345, 0.323106),
    vec3(-0.130860, 0.119294, 0.280219),
    vec3(0.154035, -0.065371, 0.229842),
    vec3(0.052938, -0.227866, 0.148478),
    vec3(-0.187305, -0.040225, 0.015926),
    vec3(0.141843, 0.047163, 0.134847),
    vec3(-0.044268, 0.055616, 0.055859),
    vec3(-0.023583, -0.080970, 0.219130),
    vec3(-0.142147, 0.198069, 0.005194),
    vec3(0.158646, 0.230457, 0.043715),
    vec3(0.030040, 0.381832, 0.163825),
    vec3(0.083006, -0.309661, 0.067413),
    vec3(0.226953, -0.235350, 0.193673),
    vec3(0.381287, 0.332041, 0.529492),
    vec3(-0.556272, 0.294715, 0.301101),
    vec3(0.424490, 0.005647, 0.117578),
    vec3(0.366500, 0.003588, 0.085702),
    vec3(0.329018, 0.030898, 0.178504),
    vec3(-0.082938, 0.512848, 0.056555),
    vec3(0.867363, -0.002734, 0.100138),
    vec3(0.455745, -0.772006, 0.003841),
    vec3(0.417290, -0.154846, 0.462514),
    vec3(-0.442722, -0.679282, 0.186503)
);

layout(std140, binding = 0) uniform SSAOParams
{
	float scale;
	float bias;
	float radius;
    float occScale;
	float attScale;
	float distScale;
	float zNear;
	float zFar;
    float OutputWidth;         
    float OutputHeight;
};


// A = proj[2][2], B = proj[3][2]
float LinearizeDepthGeneric(float depth, float A, float B) 
{
    float z = depth * 2.0 - 1.0; // Convert to NDC
    return B / (z - A);
}


float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // Convert [0,1] to NDC [-1,1]
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}


#define ROT_TEXTURE_WIDTH 4.0
#define ROT_TEXTURE_HEIGHT 4.0


void main()
{
    float Z = LinearizeDepth(texture(texDepth, TexCoords).r); 

    vec2 NoiseScale = vec2(OutputWidth / ROT_TEXTURE_WIDTH, OutputHeight / ROT_TEXTURE_HEIGHT);
   
    // Sample noise texture and build TBN matrix
    vec3 RandomVec = normalize(texture(texRotation, TexCoords * NoiseScale).xyz);
    vec3 Normal = texture(NormalMap, TexCoords).xyz; 
    vec3 Tangent = normalize(RandomVec - Normal * dot(RandomVec, Normal));
    vec3 Bitangent = cross(Normal, Tangent);
    mat3 TBN = mat3(Tangent, Bitangent, Normal);

    float att = 0.0;

    for (int i = 0; i < 64; i++) {
        vec3 ViewSpaceVec = TBN * offsets[i]; // rotate sample
        vec2 offsetUV = TexCoords + (ViewSpaceVec.xy * radius / Z);
        offsetUV = clamp(offsetUV, vec2(0.0), vec2(1.0));

        float zSampleRaw = texture(texDepth, offsetUV).r;
        float zSample = LinearizeDepth(zSampleRaw);

        float dist = max(Z - zSample, 0.0) / distScale;
        float occl = occScale * max(dist * (2.0 - dist), 0.0);

        att += 1.0 / (1.0 + occl * occl);
    }    

    att = clamp(att / 64.0, 0.0, 1.0) * attScale;
    outColor = vec4( vec3(att), 1.0 );
    //outColor = vec4(Z*5/zFar);
    //outColor = vec4(Normal, 0.0);
}
