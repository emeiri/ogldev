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

#include "ogldev_bezier_curve_technique.h"

BezierCurveTechnique::BezierCurveTechnique()
{

}


bool BezierCurveTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "../Common/Shaders/bezier_curve.vs")) {
        return false;
    }

    if (!AddShader(GL_TESS_EVALUATION_SHADER, "../Common/Shaders/bezier_curve.tes")) {
        return false;
    }

    if (!AddShader(GL_TESS_CONTROL_SHADER, "../Common/Shaders/bezier_curve.tcs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "../Common/Shaders/bezier_curve.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_wvpLoc = GetUniformLocation("gWVP");
    m_numSegmentsLoc = GetUniformLocation("gNumSegments");
    m_lineColorLoc = GetUniformLocation("gLineColor");

    return 
        ((m_wvpLoc != INVALID_UNIFORM_LOCATION) &&
        (m_numSegmentsLoc != INVALID_UNIFORM_LOCATION) &&
        (m_lineColorLoc != INVALID_UNIFORM_LOCATION));
}


void BezierCurveTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_wvpLoc, 1, GL_TRUE, (const GLfloat*)WVP.m);
}


void BezierCurveTechnique::SetNumSegments(int NumSegments)
{
    glUniform1i(m_numSegmentsLoc, NumSegments);
}


void BezierCurveTechnique::SetLineColor(float r, float g, float b, float a)
{
    glUniform4f(m_lineColorLoc, r, g, b, a);
}
