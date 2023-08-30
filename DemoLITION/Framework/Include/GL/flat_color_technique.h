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

#ifndef FLAT_COLOR_TECHNIQUE_H
#define FLAT_COLOR_TECHNIQUE_H

#include "ogldev_math_3d.h"
#include "ogldev_util.h"
#include "technique.h"


class FlatColorTechnique : public Technique
{
public:
    FlatColorTechnique() {}

    virtual bool Init();

    void SetWVP(const Matrix4f& WVP);
    void SetColor(const Vector4f& Col);

private:
    bool InitCommon();

    GLuint m_wvpLoc = INVALID_UNIFORM_LOCATION;
    GLuint m_colorLoc = INVALID_UNIFORM_LOCATION;
};

#endif