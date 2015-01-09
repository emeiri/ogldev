#version 330

out vec4 FragColor;

uniform sampler2D gPositionMap;
uniform sampler2D gNormalMap;
uniform vec2 gScreenSize;

vec2 CalcTexCoord()
{
    return gl_FragCoord.xy / gScreenSize;
}


float CalcAmbientOcclusion(vec2 TexCoord, vec3 WorldPos, vec3 Normal)
{
    vec3 OccluderPos = texture(gPositionMap, TexCoord).xyz;
    vec3 v = OccluderPos - WorldPos;
    float distance = length(v);
    v = normalize(v);
    return max(0.0, dot(Normal, v) * 1.0/(1.0 + distance));
}

void main()
{
    vec2 TexCoord = CalcTexCoord();
    vec3 WorldPos = texture(gPositionMap, TexCoord).xyz;
    vec3 Normal = normalize(texture(gNormalMap, TexCoord).xyz);

    float AO = 0.0;

    AO += CalcAmbientOcclusion(TexCoord + 0.1 * vec2(-1.0, -1.0) / WorldPos.z, WorldPos, Normal);
    AO += CalcAmbientOcclusion(TexCoord + 0.2 * vec2(1.0, -1.0) / WorldPos.z, WorldPos, Normal);
    AO += CalcAmbientOcclusion(TexCoord + 0.3 * vec2(-1.0, 1.0) / WorldPos.z, WorldPos, Normal);
    AO += CalcAmbientOcclusion(TexCoord + 0.4 * vec2(1.0, 1.0) / WorldPos.z, WorldPos, Normal);
   // AO += CalcAmbientOcclusion(TexCoord + 0.5 * vec2(-0.707, -0.707) * WorldPos.z, WorldPos, Normal);
  //  AO += CalcAmbientOcclusion(TexCoord + 0.6 * vec2(0.707, -0.707) * WorldPos.z, WorldPos, Normal);
  //  AO += CalcAmbientOcclusion(TexCoord + 0.7 * vec2(-0.707, 0.707) * WorldPos.z, WorldPos, Normal);
  //  AO += CalcAmbientOcclusion(TexCoord + 0.8 * vec2(0.707, 0.707) * WorldPos.z, WorldPos, Normal);
    
    AO /= 4.0;

    FragColor = vec4(AO);//*5.0;
}