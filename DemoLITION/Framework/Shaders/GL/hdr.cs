#version 460

layout(local_size_x = 10, local_size_y = 10) in;

layout(binding = 0) uniform sampler2D HdrTex;

layout(std430, binding = 1) buffer Result {
    float TileLuminance[]; // One per workgroup
};

shared float localLum[gl_WorkGroupSize.x * gl_WorkGroupSize.y]; 

void main() 
{
    ivec2 texCoord = ivec2(gl_GlobalInvocationID.xy);
    vec3 hdr = texelFetch(HdrTex, texCoord, 0).rgb;

    float lum = dot(hdr, vec3(0.2126, 0.7152, 0.0722));

    localLum[gl_LocalInvocationIndex] = log(lum + 0.0001);
    
    barrier();

    // Thread 0 reduces tile luminance
    if (gl_LocalInvocationIndex == 0) {

        float Sum = 0.0;
        
        uint Size = gl_WorkGroupSize.x * gl_WorkGroupSize.y;
        
        for (uint i = 0; i < Size; ++i) {
            Sum += localLum[i];
        }

        TileLuminance[gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x] = Sum;
    }
}
