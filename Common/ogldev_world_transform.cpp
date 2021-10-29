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

#include "ogldev_world_transform.h"


void WorldTrans::SetScale(float scale)
{
    m_scale = scale;
}


void WorldTrans::SetRotation(float x, float y, float z)
{
    m_rotation.x = x;
    m_rotation.y = y;
    m_rotation.z = z;
}


void WorldTrans::SetPosition(float x, float y, float z)
{
    m_pos.x = x;
    m_pos.y = y;
    m_pos.z = z;
}


void WorldTrans::SetPosition(const Vector3f& WorldPos)
{
    m_pos = WorldPos;
}


void WorldTrans::Rotate(float x, float y, float z)
{
    m_rotation.x += x;
    m_rotation.y += y;
    m_rotation.z += z;
}


Matrix4f WorldTrans::GetMatrix() const
{
    Matrix4f Scale;
    Scale.InitScaleTransform(m_scale, m_scale, m_scale);

    Matrix4f Rotation;
    Rotation.InitRotateTransform(m_rotation.x, m_rotation.y, m_rotation.z);

    Matrix4f Translation;
    Translation.InitTranslationTransform(m_pos.x, m_pos.y, m_pos.z);

    Matrix4f WorldTransformation = Translation * Rotation * Scale;

    return WorldTransformation;
}


Matrix4f WorldTrans::GetReversedTranslationMatrix() const
{
    Matrix4f ReversedTranslation;
    ReversedTranslation.InitTranslationTransform(m_pos.Negate());
    return ReversedTranslation;
}


Matrix4f WorldTrans::GetReversedRotationMatrix() const
{
    Matrix4f ReversedRotation;
    ReversedRotation.InitRotateTransformZYX(-m_rotation.x, -m_rotation.y, -m_rotation.z);
    return ReversedRotation;
}


Vector3f WorldTrans::WorldPosToLocalPos(const Vector3f& WorldPos) const
{
    Matrix4f WorldToLocalTranslation = GetReversedTranslationMatrix();
    Matrix4f WorldToLocalRotation = GetReversedRotationMatrix();
    Matrix4f WorldToLocalTransformation = WorldToLocalRotation * WorldToLocalTranslation;
    Vector4f WorldPos4f = Vector4f(WorldPos, 1.0f);
    Vector4f LocalPos4f = WorldToLocalTransformation * WorldPos4f;
    Vector3f LocalPos3f(LocalPos4f);
    return LocalPos3f;
}


Vector3f WorldTrans::WorldDirToLocalDir(const Vector3f& WorldDirection) const
{
    Matrix3f World3f(GetMatrix());  // Initialize using the top left corner

    // Inverse local-to-world transformation using transpose
    // (assuming uniform scaling)
    Matrix3f WorldToLocal = World3f.Transpose();

    Vector3f LocalDirection = WorldToLocal * WorldDirection;

    LocalDirection = LocalDirection.Normalize();

    return LocalDirection;
}
