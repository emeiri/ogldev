/*
        Copyright 2010 Etay Meiri

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

#ifndef PIPELINE_H
#define PIPELINE_H

#include "ogldev_math_3d.h"
#include "ogldev_camera.h"
#include "ogldev_world_transform.h"

struct Orientation
{
    Vector3f m_scale;
    Vector3f m_rotation;
    Vector3f m_pos;

    Orientation()
    {
        m_scale    = Vector3f(1.0f, 1.0f, 1.0f);
        m_rotation = Vector3f(0.0f, 0.0f, 0.0f);
        m_pos      = Vector3f(0.0f, 0.0f, 0.0f);
    }
};


class Pipeline
{
public:
    Pipeline()
    {
        m_scale      = Vector3f(1.0f, 1.0f, 1.0f);
        m_worldPos   = Vector3f(0.0f, 0.0f, 0.0f);
        m_rotateInfo = Vector3f(0.0f, 0.0f, 0.0f);
    }

    void Scale(float s)
    {
        Scale(s, s, s);
    }


    void Scale(const Vector3f& scale)
    {
        Scale(scale.x, scale.y, scale.z);
    }

    void Scale(float ScaleX, float ScaleY, float ScaleZ)
    {
        m_scale.x = ScaleX;
        m_scale.y = ScaleY;
        m_scale.z = ScaleZ;
    }

    void WorldPos(float x, float y, float z)
    {
        m_worldPos.x = x;
        m_worldPos.y = y;
        m_worldPos.z = z;
    }

    void WorldPos(const Vector3f& Pos)
    {
        m_worldPos = Pos;
    }

    void Rotate(float RotateX, float RotateY, float RotateZ)
    {
        m_rotateInfo.x = RotateX;
        m_rotateInfo.y = RotateY;
        m_rotateInfo.z = RotateZ;
    }

    void Rotate(const Vector3f& r)
    {
        Rotate(r.x, r.y, r.z);
    }

    void SetPerspectiveProj(const PersProjInfo& p)
    {
        m_persProjInfo = p;
    }

    void SetOrthographicProj(const OrthoProjInfo& p)
    {
        m_orthoProjInfo = p;
    }

    void SetCamera(const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up)
    {
        m_camera.Pos = Pos;
        m_camera.Target = Target;
        m_camera.Up = Up;
    }

    void SetCamera(const Camera& camera)
    {
        SetCamera(camera.GetPos(), camera.GetTarget(), camera.GetUp());
    }

    void Orient(const Orientation& o)
    {
        m_scale      = o.m_scale;
        m_worldPos   = o.m_pos;
        m_rotateInfo = o.m_rotation;
    }

    void Orient(const WorldTrans& w)
    {
        m_scale = w.GetScale();
        m_rotateInfo = w.GetRotation();
        m_worldPos = w.GetPos();
    }

    const Matrix4f& GetWPTrans();
    const Matrix4f& GetWVTrans();
    const Matrix4f& GetVPTrans();
    const Matrix4f& GetWVPTrans();
    const Matrix4f& GetWVOrthoPTrans();
    const Matrix4f& GetWorldTrans();
    const Matrix4f& GetViewTrans();
    const Matrix4f& GetProjTrans();

private:
    Vector3f m_scale;
    Vector3f m_worldPos;
    Vector3f m_rotateInfo;

    PersProjInfo m_persProjInfo;
    OrthoProjInfo m_orthoProjInfo;

    struct {
        Vector3f Pos;
        Vector3f Target;
        Vector3f Up;
    } m_camera;

    Matrix4f m_WVPtransformation;
    Matrix4f m_VPtransformation;
    Matrix4f m_WPtransformation;
    Matrix4f m_WVtransformation;
    Matrix4f m_Wtransformation;
    Matrix4f m_Vtransformation;
    Matrix4f m_ProjTransformation;
};


#endif  /* PIPELINE_H */
