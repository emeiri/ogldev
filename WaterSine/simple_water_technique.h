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

#ifndef SIMPLE_WATER_TECHNIQUE_H
#define SIMPLE_WATER_TECHNIQUE_H

#include "technique.h"
#include "ogldev_math_3d.h"

#define MAX_WAVES 4

struct WaveParam {
    float WaveLen = 0.0f;
    float Speed = 0.0f;
    float Amp = 0.0f;
    Vector2f Dir = { 0.0f, 0.0f };
};


class SimpleWaterTechnique : public Technique
{
public:

    SimpleWaterTechnique();

    virtual bool Init();

    void SetWVP(const Matrix4f& WVP);
    void SetWaterHeight(float Height);
    void SetTime(float Time);
    void SetWaveParam(int WaveIndex, const WaveParam& Wave);

private:
    GLuint m_WVPLoc = -1;
    GLuint m_heightLoc = -1;
    GLuint m_timeLoc = -1;
    struct {
        GLuint WaveLenLoc = -1;
        GLuint SpeedLoc = -1;
        GLuint AmpLoc = -1;
        GLuint DirLoc = -1;
    } m_waveParams[MAX_WAVES];
};

#endif  /* SIMPLE_WATER_TECHNIQUE_H */
