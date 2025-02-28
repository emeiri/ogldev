/*

        Copyright 2024 Etay Meiri

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


class InfiniteGridTechnique : public Technique
{
public:
    InfiniteGridTechnique() {}

    virtual bool Init();

    void SetVP(const Matrix4f& VP);
    void SetLightVP(const Matrix4f& LightVP);
    void SetCameraWorldPos(const Vector3f& CameraWorldPos);
    void SetCellSize(float CellSize);
    void SetShadowMapTextureUnit(unsigned int TextureUnit);
    void SetLightDirection(const Vector3f& LightDir);
    void ControlShadows(bool ShadowsEnabled);

private:
    bool InitCommon();

    GLuint VPLoc             = INVALID_UNIFORM_LOCATION;
    GLuint CameraWorldPosLoc = INVALID_UNIFORM_LOCATION;
    GLuint GridCellSizeLoc   = INVALID_UNIFORM_LOCATION;
    GLuint LightVPLoc        = INVALID_UNIFORM_LOCATION;
    GLuint ShadowMapLoc      = INVALID_UNIFORM_LOCATION;
    GLuint LightDirectionLoc = INVALID_UNIFORM_LOCATION;
    GLuint ShadowsEnabledLoc = INVALID_UNIFORM_LOCATION;
};