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

#include "ogldev_math_3d.h"
#include "ogldev_util.h"
#include "technique.h"


class SkyboxTechnique : public Technique
{
public:
    SkyboxTechnique() {}

    virtual bool Init();

    void SetVP(const Matrix4f& VP);
    void SetTextureUnitIndex(int TextureUnitIndex);

private:
    bool InitCommon();

    GLuint m_VPLoc = INVALID_UNIFORM_LOCATION;
    GLuint m_cubemapSamplerLoc = INVALID_UNIFORM_LOCATION;
};