#version 330

out vec4 FragColor;

uniform sampler2D gPositionMap;
uniform sampler2D gNormalMap;
uniform sampler2D gRandomMap;
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


float CalcAmbientOcclusion(vec2 TexCoord, vec3 Pos, vec3 Normal)
{
    vec3 OccluderPos = texture(gPositionMap, TexCoord).xyz;
    vec3 v = OccluderPos - Pos;
    float distance = length(v);
    v = normalize(v);

    float RangeCheck = smoothstem(0.0, 1.0, 1 / abs(OccluderPos.z - Pos.z));
    return RangeCheck * step(OccluderPos.z, 

    //return (max(0.0, dot(Normal, v))) * 
      //     (1.0/(1.0 + distance)) * 
        //   gIntensity;
}

vec2 v[4] = vec2[4](vec2(-1.0, -1.0) , 
                    vec2(1.0, -1.0) , 
                    vec2(-1.0, 1.0) , 
                    vec2(1.0, 1.0) );

void main()
{
    vec2 TexCoord = CalcTexCoord();
    vec3 Pos = texture(gPositionMap, TexCoord).xyz;
    vec3 Normal = normalize(texture(gNormalMap, TexCoord).xyz * 2.0 - 1.0);
    vec2 RandomTexCoord = TexCoord * textureSize(gPositionMap) / textureSize(gRandomMap);
    vec3 Random = texture(gRandomMap, RandomTexCoord).xyz * 2.0 - 1.0;

    float rad = gSampleRad / Pos.z;

    float AO = 0.0;

    for (int i = 0 ; i < MAX_KERNEL_SIZE ; i++) {
        vec3 sample = Pos + gKernel[i];
        vec4 offset = vec4(sample, 1.0);
        offset = gProj * offset;
        offset.xy /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;
    }
    
    FragColor = vec4(AO);
}