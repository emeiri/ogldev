#version 330

out vec4 FragColor;

uniform sampler2D gPositionMap;
uniform vec2 gScreenSize;
uniform float gIntensity;
uniform float gSampleRad;
uniform mat4 gProj;

const int MAX_KERNEL_SIZE = 128;
uniform vec3 gKernel[MAX_KERNEL_SIZE];

vec2 CalcTexCoord()
{
    return gl_FragCoord.xy / gScreenSize;
}


void main()
{
    vec2 TexCoord = CalcTexCoord();
    vec3 Pos = texture(gPositionMap, TexCoord).xyz;

    float AO = 0.0;

    for (int i = 0 ; i < MAX_KERNEL_SIZE ; i++) {
        vec3 samplePos = Pos + gKernel[i];
        vec4 offset = vec4(samplePos, 1.0);
        offset = gProj * offset;
        offset.xy /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;
            
        float sampleDepth = texture(gPositionMap, offset.xy).b;

        if (abs(Pos.z - sampleDepth) < gSampleRad) {
            AO += step(sampleDepth,samplePos.z);
           // if (sampleDepth <= samplePos.z) {
           //    AO += 1.0/128.0;
          //  }
        }
    }

    AO = 1.0 - AO/128.0;
 
    FragColor = vec4(pow(AO, 2.0));
}