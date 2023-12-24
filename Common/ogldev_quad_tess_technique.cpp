/*

        Copyright 2023 Etay Meiri

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

#include "ogldev_quad_tess_technique.h"

QuadTessTechnique::QuadTessTechnique()
{

}


bool QuadTessTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "../Common/Shaders/passthru_vec2.vs")) {
        return false;
    }

    if (!AddShader(GL_TESS_CONTROL_SHADER, "../Common/Shaders/quad_tess.tcs")) {
        return false;
    }

    if (!AddShader(GL_TESS_EVALUATION_SHADER, "../Common/Shaders/quad_tess.tes")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "../Common/Shaders/color.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_wvpLoc = GetUniformLocation("gWVP");

    m_outerLevelLeftLoc = GetUniformLocation("gOuterLevelLeft");
    m_outerLevelBottomLoc = GetUniformLocation("gOuterLevelBottom");
    m_outerLevelRightLoc = GetUniformLocation("gOuterLevelRight");
    m_outerLevelTopLoc = GetUniformLocation("gOuterLevelTop");
    
    m_innerLevelLeftRightLoc = GetUniformLocation("gInnerLevelLeftRight");
    m_innerLevelTopBottomLoc = GetUniformLocation("gInnerLevelTopBottom");
    
    m_colorLoc = GetUniformLocation("gColor");

    return 
        ((m_wvpLoc != INVALID_UNIFORM_LOCATION) &&
         (m_colorLoc != INVALID_UNIFORM_LOCATION) &&
         (m_outerLevelRightLoc != INVALID_UNIFORM_LOCATION) &&
         (m_outerLevelBottomLoc != INVALID_UNIFORM_LOCATION) &&
         (m_outerLevelLeftLoc != INVALID_UNIFORM_LOCATION) &&
         (m_outerLevelTopLoc != INVALID_UNIFORM_LOCATION) &&
         (m_innerLevelLeftRightLoc != INVALID_UNIFORM_LOCATION) &&
         (m_innerLevelTopBottomLoc != INVALID_UNIFORM_LOCATION));
}


void QuadTessTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_wvpLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


void QuadTessTechnique::SetLevels(float OuterLevelLeft,
                                  float OuterLevelBottom,
                                  float OuterLevelRight,
                                  float OuterLevelTop,
                                  float InnerLevelLeftRight,
                                  float InnerLevelTopBottom)
{
    glUniform1f(m_outerLevelLeftLoc, OuterLevelLeft);
    glUniform1f(m_outerLevelBottomLoc, OuterLevelBottom);
    glUniform1f(m_outerLevelRightLoc, OuterLevelRight);
    glUniform1f(m_outerLevelTopLoc, OuterLevelTop);

    glUniform1f(m_innerLevelLeftRightLoc, InnerLevelLeftRight);
    glUniform1f(m_innerLevelTopBottomLoc, InnerLevelTopBottom);
}


void QuadTessTechnique::SetColor(const Vector4f& Color)
{
    glUniform4f(m_colorLoc, Color.x, Color.y, Color.z, Color.w);
}


