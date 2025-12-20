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

#pragma once

#include "technique.h"
#include "ogldev_math_3d.h"


class GeometryTechnique : public Technique
{
 public:

    GeometryTechnique();

    virtual bool Init();

    void SetWVP(const Matrix4f& WVP);
    void SetNormalMatrix(const Matrix3f& NormalMatrix);
    void SetVP(const Matrix4f& WVP);            // Indirect rendering    
    void SetAlbedoTextureUnit(unsigned int TextureUnit);
    void ControlIndirectRender(bool IsRenderIndirect);
    void ControlPVP(bool IsPVP);

 private:

    DEF_LOC(m_WVPLoc);
    DEF_LOC(m_normalMatrixLoc);
    DEF_LOC(m_VPLoc);
    DEF_LOC(m_isIndirectRenderLoc);
    DEF_LOC(m_isPVPLoc);
    DEF_LOC(m_albedoSamplerLoc);
};

