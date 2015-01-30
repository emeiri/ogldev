#version 330

out vec4 FragColor;

uniform sampler2D gPositionMap;
uniform sampler2D gNormalMap;
//uniform sampler2D gRandomMap;
uniform sampler2D gDepthMap;
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


float linearizeDepth(float d)
{
    float zNear = 1.0;
    float zFar = 1000.0;
    float z_n = 2.0 * d - 1.0;
    float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
    return z_e;
    //return (gProj[3][2] / (d - gProj[2][2]));
}


void main()
{
    vec2 TexCoord = CalcTexCoord();
    vec3 Pos = texture(gPositionMap, TexCoord).xyz;
    vec3 Normal = normalize(texture(gNormalMap, TexCoord).xyz * 2.0 - 1.0);
  //  vec2 RandomTexCoord = TexCoord * textureSize(gPositionMap) / textureSize(gRandomMap);
  //  vec3 Random = texture(gRandomMap, RandomTexCoord).xyz * 2.0 - 1.0;

    float rad = gSampleRad / Pos.z;

    float AO = 0.0;

    for (int i = 0 ; i < MAX_KERNEL_SIZE ; i++) {
        vec3 samplePos = Pos + gKernel[i];
        vec4 offset = vec4(samplePos, 1.0);
        offset = gProj * offset;
        offset.xy /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;
            
        float sampleDepth = texture(gDepthMap, offset.xy).r;
        sampleDepth = linearizeDepth(sampleDepth);

        float rangeCheck = smoothstep(0.0, 1.0, 1.0 / abs(Pos.z - sampleDepth));

       // AO += rangeCheck * step(sampleDepth, samplePos.z);        
        if (sampleDepth >= samplePos.z) {
            AO += 1.0;
        }
    }
    
    AO = 1.0 - AO / 128.0;

    FragColor = vec4(pow(AO, 2.0));
}