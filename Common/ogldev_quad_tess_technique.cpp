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

   // if (!AddShader(GL_GEOMETRY_SHADER, "../Common/Shaders/wireframe_on_mesh.gs")) {
   //     return false;
   // }

    if (!AddShader(GL_FRAGMENT_SHADER, "../Common/Shaders/bezier_curve.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_wvpLoc = GetUniformLocation("gWVP");
    m_outerLevelLoc = GetUniformLocation("gOuterLevel");
    m_innerLevelLoc = GetUniformLocation("gInnerLevel");

    return 
        ((m_wvpLoc != INVALID_UNIFORM_LOCATION) &&
        (m_outerLevelLoc != INVALID_UNIFORM_LOCATION) &&
        (m_innerLevelLoc != INVALID_UNIFORM_LOCATION));
}


void QuadTessTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_wvpLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


void QuadTessTechnique::SetLevels(int OuterLevel, int InnerLevel)
{
    glUniform1i(m_outerLevelLoc, OuterLevel);
    glUniform1i(m_innerLevelLoc, InnerLevel);
}


