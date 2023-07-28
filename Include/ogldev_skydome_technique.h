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

#ifndef OGLDEV_SKYDOME_TECHNIQUE_H
#define OGLDEV_SKYDOME_TECHNIQUE_H

#include "technique.h"
#include "ogldev_math_3d.h"

class SkydomeTechnique : public Technique
{
public:

    SkydomeTechnique();

    virtual bool Init();

    void SetWVP(const Matrix4f& WVP);
    void SetRotate(const Matrix4f& Rotate);
    void SetTextureUnit(unsigned int TextureUnit);

private:

    GLuint m_WVPLoc = -1;
    GLuint m_samplerLoc = -1;
};

#endif  /* OGLDEV_SKYDOME_TECHNIQUE_H */
