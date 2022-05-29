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

#ifndef OGLDEV_SKINNING_TECHNIQUE_H
#define OGLDEV_SKINNING_TECHNIQUE_H

#include "technique.h"
#include "ogldev_math_3d.h"
#include "ogldev_new_lighting.h"


class SkinningTechnique : public LightingTechnique
{
public:

    static const unsigned int MAX_BONES = 100;

    SkinningTechnique();

    virtual bool Init();

    void SetBoneTransform(uint Index, const Matrix4f& Transform);

private:

    GLuint m_boneLocation[MAX_BONES];
};


#endif  /* OGLDEV_SKINNING_TECHNIQUE_H */
