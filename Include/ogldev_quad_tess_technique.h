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

#pragma once

#include "technique.h"
#include "ogldev_math_3d.h"


class QuadTessTechnique : public Technique
{
 public:

    QuadTessTechnique();

    virtual bool Init();

    void SetWVP(const Matrix4f& WVP);

    void SetLevels(
        float OuterLevelLeft,
        float OuterLevelBottom,
        float OuterLevelRight,
        float OuterLevelTop,
        float InnerLevelLeftRight,
        float InnerLevelTopBottom);

    void SetColor(const Vector4f& Color);

 private:

    GLuint m_wvpLoc = INVALID_UNIFORM_LOCATION;

    GLuint m_outerLevelLeftLoc = INVALID_UNIFORM_LOCATION;
    GLuint m_outerLevelBottomLoc = INVALID_UNIFORM_LOCATION;
    GLuint m_outerLevelRightLoc = INVALID_UNIFORM_LOCATION;
    GLuint m_outerLevelTopLoc = INVALID_UNIFORM_LOCATION;

    GLuint m_innerLevelLeftRightLoc = INVALID_UNIFORM_LOCATION;
    GLuint m_innerLevelTopBottomLoc = INVALID_UNIFORM_LOCATION;

    GLuint m_colorLoc = INVALID_UNIFORM_LOCATION;
};
