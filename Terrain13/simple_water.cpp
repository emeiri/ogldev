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
   // printf("%f\n", m_time);

    m_waterTech.SetTime(m_time);

    m_waterTech.SetWaveParam(0, m_waveParams[0]);
    m_waterTech.SetWaveParam(1, m_waveParams[1]);
    m_waterTech.SetWaveParam(2, m_waveParams[2]);
    m_waterTech.SetWaveParam(3, m_waveParams[3]);

    m_water.Render();

    m_prevTime = CurTime;
    if (m_time >= 8.0f) {
      //  m_time = 0.0f;
    }
}
