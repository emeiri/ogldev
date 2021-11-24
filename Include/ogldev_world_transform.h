/*

        Copyright 2021 Etay Meiri

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

#ifndef WORLD_TRANSFORM_H
#define WORLD_TRANSFORM_H

#include "ogldev_math_3d.h"

class WorldTrans {
 public:
    WorldTrans() {}

    void SetScale(float scale);
    void SetRotation(float x, float y, float z);
    void SetRotation(const Vector3f& );
    void SetPosition(float x, float y, float z);
    void SetPosition(const Vector3f& WorldPos);

    void Rotate(float x, float y, float z);

    Matrix4f GetMatrix() const;

    Vector3f WorldPosToLocalPos(const Vector3f& WorldPos) const;
    Vector3f WorldDirToLocalDir(const Vector3f& WorldDir) const;

    Matrix4f GetReversedTranslationMatrix() const;
    Matrix4f GetReversedRotationMatrix() const;

    float GetScale() const { return m_scale; }
    Vector3f GetPos() const { return m_pos; }
    Vector3f GetRotation() const { return m_rotation; }

 private:
    float    m_scale    = 1.0f;
    Vector3f m_rotation = Vector3f(0.0f, 0.0f, 0.0f);
    Vector3f m_pos      = Vector3f(0.0f, 0.0f, 0.0f);
};


#endif
