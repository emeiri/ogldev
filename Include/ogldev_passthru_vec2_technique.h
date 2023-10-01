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

#ifndef PASSTHRU_VEC2_TECHNIQUE
#define PASSTHRU_VEC2_TECHNIQUE

#include "technique.h"
#include "ogldev_math_3d.h"

class PassthruVec2Technique : public Technique
{
public:

    PassthruVec2Technique();

    virtual bool Init();

    void SetColor(float r, float g, float b);

private:
    GLuint m_colorLoc = -1;
    //    GLuint m_posLoc[4] = { -1 };
};

#endif  /* PASSTHRU_VEC2_TECHNIQUE */
