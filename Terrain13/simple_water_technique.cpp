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

#include "ogldev_util.h"
#include "simple_water_technique.h"


SimpleWaterTechnique::SimpleWaterTechnique()
{
}


bool SimpleWaterTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "simple_water.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "simple_water.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_WVPLoc = GetUniformLocation("gWVP");
    m_heightLoc = GetUniformLocation("gHeight");
    m_timeLoc = GetUniformLocation("gTime");

    if (m_WVPLoc == INVALID_UNIFORM_LOCATION ||
        m_heightLoc == INVALID_UNIFORM_LOCATION ||
        m_timeLoc == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    for (int i = 0; i < MAX_WAVES; i++) {
        char Name[128];
        snprintf(Name, sizeof(Name), "gWaveParam[%d].WaveLen", i);
        m_waveParams[i].WaveLenLoc = GetUniformLocation(Name);
        if (m_waveParams[i].WaveLenLoc == INVALID_UNIFORM_LOCATION) {
            return false;
        }

        snprintf(Name, sizeof(Name), "gWaveParam[%d].Speed", i);
        m_waveParams[i].SpeedLoc = GetUniformLocation(Name);
        if (m_waveParams[i].SpeedLoc == INVALID_UNIFORM_LOCATION) {
            return false;
        }

        snprintf(Name, sizeof(Name), "gWaveParam[%d].Amp", i);
        m_waveParams[i].AmpLoc = GetUniformLocation(Name);
        if (m_waveParams[i].AmpLoc == INVALID_UNIFORM_LOCATION) {
            return false;
        }

        snprintf(Name, sizeof(Name), "gWaveParam[%d].Dir", i);
        m_waveParams[i].DirLoc = GetUniformLocation(Name);
        if (m_waveParams[i].DirLoc == INVALID_UNIFORM_LOCATION) {
            return false;
        }
    }

    return true;
}


void SimpleWaterTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_WVPLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


void SimpleWaterTechnique::SetWaterHeight(float Height)
{
    glUniform1f(m_heightLoc, Height);
}


void SimpleWaterTechnique::SetTime(float Time)
{
    glUniform1f(m_timeLoc, Time);
}


void SimpleWaterTechnique::SetWaveParam(int WaveIndex, const WaveParam& Wave)
{
    if (WaveIndex >= MAX_WAVES) {
        printf("Invalid wave index %d\n", WaveIndex);
        exit(0);
    }

    glUniform1f(m_waveParams[WaveIndex].WaveLenLoc, Wave.WaveLen);
    glUniform1f(m_waveParams[WaveIndex].SpeedLoc, Wave.Speed);
    glUniform1f(m_waveParams[WaveIndex].AmpLoc, Wave.Amp);
    glUniform2f(m_waveParams[WaveIndex].DirLoc, Wave.Dir.x, Wave.Dir.y);
}