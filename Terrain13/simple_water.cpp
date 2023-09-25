/*
    Copyright 2022 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "simple_water.h"
#include "texture_config.h"


static float RandZeroToOne()
{
    return float(double(rand()) / double(RAND_MAX));
}


SimpleWater::SimpleWater()
{    
}


SimpleWater::~SimpleWater()
{
}


void SimpleWater::Init(int Size, float WorldScale)
{
    if (!m_waterTech.Init()) {
        printf("Error initializing water tech\n");
        exit(0);
    }

    m_waterTech.Enable();
    m_waterTech.SetWaterHeight(m_waterHeight);

    m_water.CreateTriangleList(Size, Size, WorldScale);

    m_prevTime = GetCurrentTimeMillis();

    InitWaves();
}


void SimpleWater::SetWaveParam(int WaveIndex, const WaveParam& Wave)
{
    assert(WaveIndex < MAX_WAVES);

    m_waveParams[WaveIndex] = Wave;
}


void SimpleWater::Render(const Matrix4f& WVP)
{
    m_waterTech.Enable();
    m_waterTech.SetWVP(WVP);
    m_waterTech.SetWaterHeight(m_waterHeight);

    long long CurTime = GetCurrentTimeMillis();
    long long DeltaTime = CurTime - m_prevTime;
    m_time += DeltaTime / 1000.0f;
   // printf("%f\n", DeltaTime / 1000.0f);

    m_waterTech.SetTime(m_time);

    m_waterTech.SetWaveParam(0, m_waveParams[0]);
    m_waterTech.SetWaveParam(1, m_waveParams[1]);
    m_waterTech.SetWaveParam(2, m_waveParams[2]);
    m_waterTech.SetWaveParam(3, m_waveParams[3]);

    m_water.Render();

    m_prevTime = CurTime;

    UpdateWaves((int)DeltaTime);
}

float medianWavelength = 50.0f;
float wavelengthRange = 1.0f;
float medianDirection = 0.0f;
float directionalRange = 30.0f;
float medianAmplitude = 10.0f;
float steepness = 0.0f;
float wavelengthMin = medianWavelength / (1.0f + wavelengthRange);

float wavelengthMax = medianWavelength * (1.0f + wavelengthRange);
float directionMin = medianDirection - directionalRange;
float directionMax = medianDirection + directionalRange;
float ampOverLen = medianAmplitude / medianWavelength;


void SimpleWater::InitWaves()
{
    for (int i = 0; i < MAX_WAVES; i++) {
        float wavelength = RandZeroToOne() * (wavelengthMax - wavelengthMin) + wavelengthMin;
        float direction = RandZeroToOne() * (directionMax - directionMin) + directionMin;
        float amplitude = wavelength * ampOverLen;
        float speed = sqrtf(1000.0f * (float)M_PI / wavelength);

        m_waveParams[i].WaveLen = wavelength;
        m_waveParams[i].Dir = Vector2f(cosf(ToRadian(direction)), sinf(ToRadian(direction)));
       // m_waveParams[i].Dir = Vector2f(1.0f, 0.0f);
        m_waveParams[i].Dir.Normalize();
        m_waveParams[i].Amp = amplitude;
        m_waveParams[i].Speed = speed;

        printf("speed %f\n", speed);
        printf("amp %f\n", amplitude);
        printf("dir %f,%f\n", m_waveParams[i].Dir.x, m_waveParams[i].Dir.y);
    }
}


void SimpleWater::UpdateWaves(int DeltaTimeMillis)
{
    m_fadeTime += DeltaTimeMillis;
    if (m_fadeTime >= 2000) {
        float wavelength = RandZeroToOne() * (wavelengthMax - wavelengthMin) + wavelengthMin;
        float direction = RandZeroToOne() * (directionMax - directionMin) + directionMin;
        float amplitude = wavelength * ampOverLen;
        float speed = sqrtf(1000.0f * (float)M_PI / wavelength);

     //   m_waveParams[m_transIndex].WaveLen += 0.1f;
        m_waveParams[m_transIndex].Dir = Vector2f(cosf(ToRadian(direction)), sinf(ToRadian(direction)));
      //  m_waveParams[m_transIndex].Dir = Vector2f(1.0f, 0.0f);
        m_waveParams[m_transIndex].Dir.Normalize();
        m_waveParams[m_transIndex].Amp += 0.1f;
        m_waveParams[m_transIndex].Speed += 0.1f;

        m_fadeTime = 0;
        m_transIndex++;
        m_transIndex = m_transIndex % MAX_WAVES;
        printf("New wave %d\n", m_transIndex);
    }
}