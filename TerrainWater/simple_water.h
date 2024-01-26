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

#ifndef SIMPLE_WATER_H
#define SIMPLE_WATER_H

#include "ogldev_texture.h"
#include "simple_water_technique.h"
#include "triangle_list.h"


class SimpleWater {
 public:

    SimpleWater();

    ~SimpleWater();

    void Init(int Size, float WorldScale);

    void SetWaterHeight(float Height) { m_waterHeight = Height; }

    void SetWaveParam(int WaveIndex, const WaveParam& Wave);

    float GetWaterHeight() const { return m_waterHeight; }

    void Render(const Matrix4f& WVP);

 private:
    void InitWaves();
    void UpdateWaves(int DeltaTimeMillis);

    TriangleList m_water;
    SimpleWaterTechnique m_waterTech;
    float m_waterHeight = 64.0f;
    long long m_prevTime = 0;
    float m_time = 0.0f;
    WaveParam m_waveParams[MAX_WAVES];
    struct {

    };
    float m_fade[MAX_WAVES] = { 0 };
    int m_transIndex = 0;
    int m_fadeTime = 0;
    float m_transDel = -1.0f / 6.0f;
    float m_minWaveLen = 15.0f;
    float m_maxWaveLen = 125.0f;
    float m_ampOverLen = 1.0f;
};

#endif
