

#version 460

layout(local_size_x = 10, local_size_y = 10) in;

layout(binding = 0) uniform sampler2D hdrTex;

uniform int numGroupsX = 192;

layout(std430, binding = 1) buffer Result {
    float tileLuminance[]; // One per workgroup
};

shared float localLum[100]; // 16x16 threads

void main() {
    ivec2 texCoord = ivec2(gl_GlobalInvocationID.xy);
    vec3 hdr = texelFetch(hdrTex, texCoord, 0).rgb;
    float lum = dot(hdr, vec3(0.2126, 0.7152, 0.0722));
    localLum[gl_LocalInvocationIndex] = log(lum + 0.00001);

    barrier();

    // Thread 0 reduces tile luminance
    if (gl_LocalInvocationIndex == 0) {
        float sum = 0.0;
        for (int i = 0; i < 100; ++i) {
            sum += localLum[i];
        }
        tileLuminance[gl_WorkGroupID.x + gl_WorkGroupID.y * numGroupsX] = sum;
    }
}// Compute shader
