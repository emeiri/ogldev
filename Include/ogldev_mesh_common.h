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


#ifndef OGLDEV_MESH_COMMON_H
#define OGLDEV_MESH_COMMON_H

class IRenderCallbacks
{
public:
    virtual void DrawStartCB(uint DrawIndex) = 0;
};


class MeshCommon {

 public:
    MeshCommon() {}

    void SetScale(float Scale);
    void SetRotation(float x, float y, float z);
    void SetPosition(float x, float y, float z);

    Vector3f GetPosition() const;

    virtual void Render(IRenderCallbacks* pRenderCallbacks = NULL) = 0;

    WorldTrans& GetWorldTransform() { return m_worldTransform; }

 protected:
    WorldTrans m_worldTransform;
};


inline void MeshCommon::SetScale(float Scale)
{
    m_worldTransform.SetScale(Scale);
}


inline void MeshCommon::SetRotation(float x, float y, float z)
{
    m_worldTransform.SetRotation(x, y, z);
}


inline void MeshCommon::SetPosition(float x, float y, float z)
{
    m_worldTransform.SetPosition(x, y, z);
}


inline Vector3f MeshCommon::GetPosition() const
{
    return m_worldTransform.GetPos();
}

#endif
