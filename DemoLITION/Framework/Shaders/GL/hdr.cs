

#version 460

layout(local_size_x = 10, local_size_y = 10) in;

layout(binding = 0) uniform sampler2D hdrTex;

uniform int numGroupsX = 192;

layout(std430, binding = 1) buffer Result {
    float tileLuminance[]; // One per workgroup
};

shared float localLum[100]; // 16x16 threads

void main() 
{
    ivec2 texCoord = ivec2(gl_GlobalInvocationID.xy);
    vec3 hdr = texelFetch(hdrTex, texCoord, 0).rgb;
    float lum = dot(hdr, vec3(0.2126, 0.7152, 0.0722));

    //float brightThreshold = 2.5;
  //  if (lum < brightThreshold) {
        //FragLum = log(luminance + 0.0001);
        localLum[gl_LocalInvocationIndex] = log(lum + 0.0001);
   // }
  //  else
  //  {
   //     localLum[gl_LocalInvocationIndex] = 0.0;
  //  }
    

    barrier();

    // Thread 0 reduces tile luminance
    if (gl_LocalInvocationIndex == 0) {
        float sum = 0.0;
        for (int i = 0; i < 100; ++i) {
            sum += localLum[i];
        }

        // for tiled tone mapping
        // float avgLogLum = sum / 100.0f;
        // float Exposure = 0.18f / exp(avgLogLum);
        // Exposure = clamp(Exposure, 0.001f, 1.0f);		
        // tileLuminance[gl_WorkGroupID.y * numGroupsX + gl_WorkGroupID.x] = Exposure;
		
		// regular
        tileLuminance[gl_WorkGroupID.y * numGroupsX + gl_WorkGroupID.x] = sum;
    }
}// Compute shader
