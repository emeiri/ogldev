/*
    Copyright 2024 Etay Meiri

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

class RenderTechnique : public Technique
{
public:

    RenderTechnique();

    virtual bool Init();

    void SetWorldMatrix(const Matrix4f& m);    

    void SetViewMatrix(const Matrix4f& m);

    void SetProjectionMatrix(const Matrix4f& m);

    void SetNormalMatrix(const Matrix4f& m);
    
private:

    GLuint WorldMatrixLoc = INVALID_UNIFORM_LOCATION;
    GLuint ViewMatrixLoc = INVALID_UNIFORM_LOCATION;
    GLuint ProjectionMatrixLoc = INVALID_UNIFORM_LOCATION;
    GLuint NormalMatrixLoc = INVALID_UNIFORM_LOCATION;
};

