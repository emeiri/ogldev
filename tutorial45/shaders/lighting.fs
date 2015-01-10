#version 330

out vec4 FragColor;

uniform sampler2D gPositionMap;
uniform sampler2D gNormalMap;
uniform vec2 gScreenSize;
uniform float gIntensity;
uniform float gSampleRad;

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

    return (max(0.0, dot(Normal, v))) * 
           (1.0/(1.0 + distance)) * 
           gIntensity;
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

    float rad = gSampleRad / Pos.z;

    float AO = 0.0;

    for (int i = 0 ; i < 4 ; i++) {
        AO += CalcAmbientOcclusion(TexCoord + v[i] * rad, Pos, Normal);
    }
    
    AO /= 4.0;

    FragColor = vec4(AO);
}