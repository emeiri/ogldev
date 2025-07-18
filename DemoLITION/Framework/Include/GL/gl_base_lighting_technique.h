/*

        Copyright 2011 Etay Meiri

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
#include "ogldev_material.h"
#include "ogldev_world_transform.h"
#include "demolition_lights.h"
#include "Int/core_model.h"

class BaseLightingTechnique : public Technique
{
public:

    BaseLightingTechnique();

    virtual bool Init();

    virtual void SetMaterial(const Material& material) = 0;
	
    void SetWVP(const Matrix4f& WVP);
    void SetWorldMatrix(const Matrix4f& WVP);
    void SetNormalMatrix(const Matrix3f& NormalMatrix);
    void SetLightWVP(const Matrix4f& LightWVP); // required only for shadow mapping
    void SetLightVP(const Matrix4f& LightVP);
    void SetVP(const Matrix4f& VP);
    void SetCameraWorldPos(const Vector3f& CameraWorldPos);
    void ControlNormalMap(bool Enable);
    void ControlParallaxMap(bool Enable);
    void ControlShadows(bool ShadowsEnabled);
    void ControlIndirectRender(bool IsRenderIndirect);
    void ControlPVP(bool IsPVP);
    void SetNumLights(int NumLights);

protected:

    bool InitCommon();

private:

    GLuint WVPLoc = INVALID_UNIFORM_LOCATION;
    GLuint WorldMatrixLoc = INVALID_UNIFORM_LOCATION;
    GLuint NormalMatrixLoc = INVALID_UNIFORM_LOCATION;
    GLuint LightWVPLoc = INVALID_UNIFORM_LOCATION; // required only for shadow mapping
    GLuint LightVPLoc = INVALID_UNIFORM_LOCATION;  // required only for shadow mapping with indirect rendering
    GLuint HasNormalMapLoc = INVALID_UNIFORM_LOCATION;
    GLuint HasHeightMapLoc = INVALID_UNIFORM_LOCATION;
    GLuint CameraWorldPosLoc = INVALID_UNIFORM_LOCATION;
    GLuint ShadowsEnabledLoc = INVALID_UNIFORM_LOCATION;
    GLuint IsIndirectRenderLoc = INVALID_UNIFORM_LOCATION;
    GLuint IsPVPLoc = INVALID_UNIFORM_LOCATION;
    GLuint VPLoc = INVALID_UNIFORM_LOCATION;
    GLuint NumLightsLoc = INVALID_UNIFORM_LOCATION;
};

