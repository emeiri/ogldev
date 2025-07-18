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

//#define FAIL_ON_MISSING_LOC


#include "GL/gl_base_lighting_technique.h"

BaseLightingTechnique::BaseLightingTechnique()
{
}


bool BaseLightingTechnique::Init()
{
    GET_UNIFORM_AND_CHECK(WVPLoc, "gWVP");
    GET_UNIFORM_AND_CHECK(WorldMatrixLoc, "gWorld");
    GET_UNIFORM_AND_CHECK(NormalMatrixLoc, "gNormalMatrix");
    GET_UNIFORM_AND_CHECK(LightWVPLoc, "gLightWVP");
    GET_UNIFORM_AND_CHECK(HasNormalMapLoc, "gHasNormalMap");
    GET_UNIFORM_AND_CHECK(CameraWorldPosLoc, "gCameraWorldPos");
    GET_UNIFORM_AND_CHECK(ShadowsEnabledLoc, "gShadowsEnabled");
    GET_UNIFORM_AND_CHECK(IsIndirectRenderLoc, "gIsIndirectRender");
    GET_UNIFORM_AND_CHECK(IsPVPLoc, "gIsPVP");
    GET_UNIFORM_AND_CHECK(VPLoc, "gVP");
    GET_UNIFORM_AND_CHECK(LightVPLoc, "gLightVP");
    GET_UNIFORM_AND_CHECK(NumLightsLoc, "gNumLights");

    return true;
}


void BaseLightingTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(WVPLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


void BaseLightingTechnique::SetWorldMatrix(const Matrix4f& World)
{
    glUniformMatrix4fv(WorldMatrixLoc, 1, GL_TRUE, (const GLfloat*)World.m);
}


void BaseLightingTechnique::SetNormalMatrix(const Matrix3f& NormalMatrix)
{
    glUniformMatrix3fv(NormalMatrixLoc, 1, GL_TRUE, (const GLfloat*)NormalMatrix.m);
}


void BaseLightingTechnique::SetLightWVP(const Matrix4f& LightWVP)
{
    glUniformMatrix4fv(LightWVPLoc, 1, GL_TRUE, (const GLfloat*)LightWVP.m);
}


void BaseLightingTechnique::ControlNormalMap(bool Enable)
{
    glUniform1i(HasNormalMapLoc, Enable);
}


void BaseLightingTechnique::ControlParallaxMap(bool Enable)
{
    glUniform1i(HasHeightMapLoc, Enable);
}


void BaseLightingTechnique::SetCameraWorldPos(const Vector3f& CameraWorldPos)
{
    glUniform3f(CameraWorldPosLoc, CameraWorldPos.x, CameraWorldPos.y, CameraWorldPos.z);
}


void BaseLightingTechnique::ControlShadows(bool ShadowsEnabled)
{
    glUniform1i(ShadowsEnabledLoc, ShadowsEnabled);
}


void BaseLightingTechnique::ControlIndirectRender(bool IsIndirectRender)
{
    glUniform1i(IsIndirectRenderLoc, IsIndirectRender);
}


void BaseLightingTechnique::ControlPVP(bool IsPVP)
{
    glUniform1i(IsPVPLoc, IsPVP);
}



void BaseLightingTechnique::SetVP(const Matrix4f& VP)
{
    glUniformMatrix4fv(VPLoc, 1, GL_TRUE, (const GLfloat*)VP.m);
}


void BaseLightingTechnique::SetLightVP(const Matrix4f& LightVP)
{
    glUniformMatrix4fv(LightVPLoc, 1, GL_TRUE, (const GLfloat*)LightVP.m);
}


void BaseLightingTechnique::SetNumLights(int NumLights)
{
    glUniform1i(NumLightsLoc, NumLights);
}


