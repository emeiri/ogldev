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
};

uniform Wave gWaveParam[4];

out vec2 oTex;
out float oHeight;
out float oMaxHeight;

float CalcSineFunc(float WaveLen, float Speed, float Amp)
{
    float Freq = 2.0 / WaveLen;
    float Phase = Speed * Freq;
    float Sine = Amp * sin(Position.x * Freq + Phase * gTime);
    return Sine;
}

void main()
{
    float Sine1 = CalcSineFunc(gWaveParam[0].WaveLen, gWaveParam[0].Speed, gWaveParam[0].Amp);
    float Sine2 = CalcSineFunc(gWaveParam[1].WaveLen, gWaveParam[1].Speed, gWaveParam[1].Amp);
    float Sine3 = CalcSineFunc(gWaveParam[2].WaveLen, gWaveParam[2].Speed, gWaveParam[2].Amp);
    float Sine4 = CalcSineFunc(gWaveParam[3].WaveLen, gWaveParam[3].Speed, gWaveParam[3].Amp);

    float Height = gHeight + Sine1 + Sine2 + Sine3 + Sine4;
   
    vec3 NewPosition = (Position + vec3(0.0, Height, 0.0));
    gl_Position = gWVP * vec4(NewPosition, 1.0);
    oTex = TexCoord;
    oHeight = Sine1 + Sine2 + Sine3 + Sine4;
    oMaxHeight = gWaveParam[0].Amp + gWaveParam[1].Amp + gWaveParam[2].Amp + gWaveParam[3].Amp;
}
