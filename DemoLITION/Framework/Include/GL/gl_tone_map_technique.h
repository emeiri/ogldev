/*
    Copyright 2025 Etay Meiri

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

#pragma once

#include "technique.h"
#include "ogldev_math_3d.h"
#include "demolition_scene.h"

class ToneMapTechnique : public Technique
{
public:

    ToneMapTechnique() {}

    virtual bool Init();

    void Render();

    void SetAverageLuminance(float AvgLum);

    void SetHDRSampler(unsigned int TextureUnit);

    void SetExposure(float Exposure);

    void SetToneMapMethod(TONE_MAP_METHOD Method);

    void ControlGammaCorrection(bool Enable);

private:

    GLuint m_dummyVAO;

    DEF_LOC(m_avgLumLoc);
    DEF_LOC(m_hdrSamplerLoc);
    DEF_LOC(m_exposure);
    DEF_LOC(m_methodTypeLoc);
    DEF_LOC(m_enableGammaLoc);
};

