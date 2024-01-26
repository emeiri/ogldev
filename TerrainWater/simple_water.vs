#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;

uniform mat4 gWVP;
uniform float gHeight = 0.0f;
uniform float gTime = 0.0f;

struct Wave {
    float WaveLen;
    float Speed;
    float Amp;
    vec2 Dir;
};

uniform Wave gWaveParam[4];

out vec2 oTex;
out float oHeight;
out float oMaxHeight;
out vec3 oNormal;

float CalcSineFunc(Wave w)
{
    float Freq = 2.0 / w.WaveLen;
    float Phase = w.Speed * Freq;
    vec3 Dir = vec3(w.Dir.x, 0.0, w.Dir.y);
    float xz = dot(Dir, Position);
    float Sine = w.Amp * sin(xz * Freq + Phase * gTime);    
    return Sine;
}

vec3 CalcNormal(Wave w)
{
    float Freq = 2.0 / w.WaveLen;
    float Phase = w.Speed * Freq;
    float x = w.Dir.x * Position.x;
    float z = w.Dir.y * Position.z;
    vec3 Dir = vec3(w.Dir.x, 0.0, w.Dir.y);
    float xz = dot(Dir, Position);
    float dx = Freq * w.Amp * cos(xz * Freq + Phase * gTime);
    float dz = Freq * w.Amp * cos(xz * Freq + Phase * gTime);
    vec3 Tangent = normalize(vec3(dx, 0, 1));
    vec3 Binormal = normalize(vec3(1, 0, dz));
    vec3 Normal = cross(Binormal, Tangent);
    Normal = normalize(Normal);
    return Normal;
}

void main()
{
    float Sine1 = CalcSineFunc(gWaveParam[0]);
    vec3 Normal1 = CalcNormal(gWaveParam[0]);

    float Sine2 = CalcSineFunc(gWaveParam[1]);
    vec3 Normal2 = CalcNormal(gWaveParam[1]);

    float Sine3 = CalcSineFunc(gWaveParam[2]);
    vec3 Normal3 = CalcNormal(gWaveParam[2]);

    float Sine4 = CalcSineFunc(gWaveParam[3]);
    vec3 Normal4 = CalcNormal(gWaveParam[3]);

    float Height = gHeight + Sine1 + Sine2 + Sine3 + Sine4;
   
    vec3 NewPosition = (Position + vec3(0.0, Height, 0.0));
    gl_Position = gWVP * vec4(NewPosition, 1.0);
    oTex = TexCoord;
    oHeight = Sine1 + Sine2 + Sine3 + Sine4;
    oMaxHeight = gWaveParam[0].Amp + gWaveParam[1].Amp + gWaveParam[2].Amp + gWaveParam[3].Amp;
    oNormal = Normal1 + Normal2 + Normal3 + Normal4;
}
