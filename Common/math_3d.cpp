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

// #define USE_GLM

#ifdef USE_GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/string_cast.hpp>
#endif
#include <iostream>
#include <stdlib.h>

#include "ogldev_util.h"
#include "ogldev_math_3d.h"


Vector4f& Vector4f::Normalize()
{
    float len = Length();

    assert(len != 0);

    x /= len;
    y /= len;
    z /= len;
    w /= len;

    return *this;
}

Vector3f Vector3f::Cross(const Vector3f& v) const
{
    const float _x = y * v.z - z * v.y;
    const float _y = z * v.x - x * v.z;
    const float _z = x * v.y - y * v.x;

    return Vector3f(_x, _y, _z);
}

Vector3f& Vector3f::Normalize()
{
    float len = Length();

    assert(len != 0);

    x /= len;
    y /= len;
    z /= len;

    return *this;
}

void Vector3f::Rotate(float Angle, const Vector3f& V)
{
    Quaternion RotationQ(Angle, V);

    Quaternion ConjugateQ = RotationQ.Conjugate();

    Quaternion W = RotationQ * (*this) * ConjugateQ;

    x = W.x;
    y = W.y;
    z = W.z;
}


Vector3f Vector3f::Negate() const
{
    Vector3f ret(-x, -y, -z);
    return ret;
}


void Matrix4f::InitScaleTransform(float ScaleX, float ScaleY, float ScaleZ)
{
    m[0][0] = ScaleX; m[0][1] = 0.0f;   m[0][2] = 0.0f;   m[0][3] = 0.0f;
    m[1][0] = 0.0f;   m[1][1] = ScaleY; m[1][2] = 0.0f;   m[1][3] = 0.0f;
    m[2][0] = 0.0f;   m[2][1] = 0.0f;   m[2][2] = ScaleZ; m[2][3] = 0.0f;
    m[3][0] = 0.0f;   m[3][1] = 0.0f;   m[3][2] = 0.0f;   m[3][3] = 1.0f;
}


void Matrix4f::InitScaleTransform(float Scale)
{
    InitScaleTransform(Scale, Scale, Scale);
}


void Matrix4f::InitScaleTransform(const Vector3f& Scale)
{
    InitScaleTransform(Scale.x, Scale.y, Scale.z);
}


void Matrix4f::InitRotateTransform(float RotateX, float RotateY, float RotateZ)
{
    Matrix4f rx, ry, rz;

    float x = ToRadian(RotateX);
    float y = ToRadian(RotateY);
    float z = ToRadian(RotateZ);

    rx.InitRotationX(x);
    ry.InitRotationY(y);
    rz.InitRotationZ(z);

    *this = rz * ry * rx;
}


void Matrix4f::InitRotateTransformZYX(float RotateX, float RotateY, float RotateZ)
{
    Matrix4f rx, ry, rz;

    float x = ToRadian(RotateX);
    float y = ToRadian(RotateY);
    float z = ToRadian(RotateZ);

    rx.InitRotationX(x);
    ry.InitRotationY(y);
    rz.InitRotationZ(z);

    *this = rx * ry * rz;
}


void Matrix4f::InitRotateTransform(const Vector3f& Rotate)
{
    InitRotateTransform(Rotate.x, Rotate.y, Rotate.z);
}


//
// The following rotation matrices are for a left handed coordinate system.
// https://butterflyofdream.wordpress.com/2016/07/05/converting-rotation-matrices-of-left-handed-coordinate-system/
//

void Matrix4f::InitRotationX(float x)
{
    m[0][0] = 1.0f; m[0][1] = 0.0f   ;  m[0][2] = 0.0f    ; m[0][3] = 0.0f;
    m[1][0] = 0.0f; m[1][1] = cosf(x);  m[1][2] = sinf(x);  m[1][3] = 0.0f;
    m[2][0] = 0.0f; m[2][1] = -sinf(x); m[2][2] = cosf(x) ; m[2][3] = 0.0f;
    m[3][0] = 0.0f; m[3][1] = 0.0f   ;  m[3][2] = 0.0f    ; m[3][3] = 1.0f;
}


void Matrix4f::InitRotationY(float y)
{
    m[0][0] = cosf(y); m[0][1] = 0.0f; m[0][2] = -sinf(y); m[0][3] = 0.0f;
    m[1][0] = 0.0f   ; m[1][1] = 1.0f; m[1][2] = 0.0f    ; m[1][3] = 0.0f;
    m[2][0] = sinf(y); m[2][1] = 0.0f; m[2][2] = cosf(y) ; m[2][3] = 0.0f;
    m[3][0] = 0.0f   ; m[3][1] = 0.0f; m[3][2] = 0.0f    ; m[3][3] = 1.0f;
}


void Matrix4f::InitRotationZ(float z)
{
    m[0][0] = cosf(z);  m[0][1] = sinf(z);  m[0][2] = 0.0f; m[0][3] = 0.0f;
    m[1][0] = -sinf(z); m[1][1] = cosf(z) ; m[1][2] = 0.0f; m[1][3] = 0.0f;
    m[2][0] = 0.0f   ;  m[2][1] = 0.0f    ; m[2][2] = 1.0f; m[2][3] = 0.0f;
    m[3][0] = 0.0f   ;  m[3][1] = 0.0f    ; m[3][2] = 0.0f; m[3][3] = 1.0f;
}


void Matrix4f::InitRotateTransform(const Quaternion& quat)
{
    float yy2 = 2.0f * quat.y * quat.y;
    float xy2 = 2.0f * quat.x * quat.y;
    float xz2 = 2.0f * quat.x * quat.z;
    float yz2 = 2.0f * quat.y * quat.z;
    float zz2 = 2.0f * quat.z * quat.z;
    float wz2 = 2.0f * quat.w * quat.z;
    float wy2 = 2.0f * quat.w * quat.y;
    float wx2 = 2.0f * quat.w * quat.x;
    float xx2 = 2.0f * quat.x * quat.x;
    m[0][0] = - yy2 - zz2 + 1.0f;
    m[0][1] = xy2 + wz2;
    m[0][2] = xz2 - wy2;
    m[0][3] = 0;
    m[1][0] = xy2 - wz2;
    m[1][1] = - xx2 - zz2 + 1.0f;
    m[1][2] = yz2 + wx2;
    m[1][3] = 0;
    m[2][0] = xz2 + wy2;
    m[2][1] = yz2 - wx2;
    m[2][2] = - xx2 - yy2 + 1.0f;
    m[2][3] = 0.0f;
    m[3][0] = m[3][1] = m[3][2] = 0;
    m[3][3] = 1.0f;
}


void Matrix4f::InitTranslationTransform(float x, float y, float z)
{
    m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = x;
    m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = y;
    m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = z;
    m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
}


void Matrix4f::InitTranslationTransform(const Vector3f& Pos)
{
    InitTranslationTransform(Pos.x, Pos.y, Pos.z);
}


void Matrix4f::InitCameraTransform(const Vector3f& Target, const Vector3f& Up)
{
    Vector3f N = Target;
    N.Normalize();

    Vector3f UpNorm = Up;
    UpNorm.Normalize();

    Vector3f U;
    U = UpNorm.Cross(N);
    U.Normalize();

    Vector3f V = N.Cross(U);

    m[0][0] = U.x;   m[0][1] = U.y;   m[0][2] = U.z;   m[0][3] = 0.0f;
    m[1][0] = V.x;   m[1][1] = V.y;   m[1][2] = V.z;   m[1][3] = 0.0f;
    m[2][0] = N.x;   m[2][1] = N.y;   m[2][2] = N.z;   m[2][3] = 0.0f;
    m[3][0] = 0.0f;  m[3][1] = 0.0f;  m[3][2] = 0.0f;  m[3][3] = 1.0f;
}


void Matrix4f::InitCameraTransform(const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up)
{
    Matrix4f CameraTranslation;
    CameraTranslation.InitTranslationTransform(-Pos.x, -Pos.y, -Pos.z);

    Matrix4f CameraRotateTrans;
    CameraRotateTrans.InitCameraTransform(Target, Up);

    *this = CameraRotateTrans * CameraTranslation;
}

void Matrix4f::InitPersProjTransform(const PersProjInfo& p)
{
    float ar         = p.Height / p.Width;
    float zRange     = p.zNear - p.zFar;
    float tanHalfFOV = tanf(ToRadian(p.FOV / 2.0f));

    m[0][0] = 1/tanHalfFOV; m[0][1] = 0.0f;                 m[0][2] = 0.0f;                        m[0][3] = 0.0;
    m[1][0] = 0.0f;         m[1][1] = 1.0f/(tanHalfFOV*ar); m[1][2] = 0.0f;                        m[1][3] = 0.0;
    m[2][0] = 0.0f;         m[2][1] = 0.0f;                 m[2][2] = (-p.zNear - p.zFar)/zRange ; m[2][3] = 2.0f*p.zFar*p.zNear/zRange;
    m[3][0] = 0.0f;         m[3][1] = 0.0f;                 m[3][2] = 1.0f;                        m[3][3] = 0.0;

#ifdef USE_GLM
    glm::mat4 Projection = glm::perspectiveFovLH(glm::radians(p.FOV), p.Width, p.Height, p.zNear, p.zFar);

    m[0][0] = Projection[0][0]; m[0][1] = Projection[1][0]; m[0][2] = Projection[2][0]; m[0][3] = Projection[3][0];
    m[1][0] = Projection[0][1]; m[1][1] = Projection[1][1]; m[1][2] = Projection[2][1]; m[1][3] = Projection[3][1];
    m[2][0] = Projection[0][2]; m[2][1] = Projection[1][2]; m[2][2] = Projection[2][2]; m[2][3] = Projection[3][2];
    m[3][0] = Projection[0][3]; m[3][1] = Projection[1][3]; m[3][2] = Projection[2][3]; m[3][3] = Projection[3][3];
#endif

    //    std::cout << glm::to_string(Projection) << std::endl;
}


void Matrix4f::InitOrthoProjTransform(const OrthoProjInfo& p)
{
    float l = p.l;
    float r = p.r;
    float b = p.b;
    float t = p.t;
    float n = p.n;
    float f = p.f;

    m[0][0] = 2.0f/(r - l); m[0][1] = 0.0f;         m[0][2] = 0.0f;         m[0][3] = -(r + l)/(r - l);
    m[1][0] = 0.0f;         m[1][1] = 2.0f/(t - b); m[1][2] = 0.0f;         m[1][3] = -(t + b)/(t - b);
    m[2][0] = 0.0f;         m[2][1] = 0.0f;         m[2][2] = 2.0f/(f - n); m[2][3] = -(f + n)/(f - n);
    m[3][0] = 0.0f;         m[3][1] = 0.0f;         m[3][2] = 0.0f;         m[3][3] = 1.0;
}


float Matrix4f::Determinant() const
{
        return m[0][0]*m[1][1]*m[2][2]*m[3][3] - m[0][0]*m[1][1]*m[2][3]*m[3][2] + m[0][0]*m[1][2]*m[2][3]*m[3][1] - m[0][0]*m[1][2]*m[2][1]*m[3][3]
                + m[0][0]*m[1][3]*m[2][1]*m[3][2] - m[0][0]*m[1][3]*m[2][2]*m[3][1] - m[0][1]*m[1][2]*m[2][3]*m[3][0] + m[0][1]*m[1][2]*m[2][0]*m[3][3]
                - m[0][1]*m[1][3]*m[2][0]*m[3][2] + m[0][1]*m[1][3]*m[2][2]*m[3][0] - m[0][1]*m[1][0]*m[2][2]*m[3][3] + m[0][1]*m[1][0]*m[2][3]*m[3][2]
                + m[0][2]*m[1][3]*m[2][0]*m[3][1] - m[0][2]*m[1][3]*m[2][1]*m[3][0] + m[0][2]*m[1][0]*m[2][1]*m[3][3] - m[0][2]*m[1][0]*m[2][3]*m[3][1]
                + m[0][2]*m[1][1]*m[2][3]*m[3][0] - m[0][2]*m[1][1]*m[2][0]*m[3][3] - m[0][3]*m[1][0]*m[2][1]*m[3][2] + m[0][3]*m[1][0]*m[2][2]*m[3][1]
                - m[0][3]*m[1][1]*m[2][2]*m[3][0] + m[0][3]*m[1][1]*m[2][0]*m[3][2] - m[0][3]*m[1][2]*m[2][0]*m[3][1] + m[0][3]*m[1][2]*m[2][1]*m[3][0];
}


Matrix4f Matrix4f::Inverse() const
{
        // Compute the reciprocal determinant
        float det = Determinant();

        if(det == 0.0f)
        {
            assert(0);
            return *this;
        }

        float invdet = 1.0f / det;

        Matrix4f res;
        res.m[0][0] = invdet  * (m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) + m[1][2] *
                                 (m[2][3] * m[3][1] - m[2][1] * m[3][3]) + m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]));
        res.m[0][1] = -invdet * (m[0][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) + m[0][2] *
                                 (m[2][3] * m[3][1] - m[2][1] * m[3][3]) + m[0][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]));
        res.m[0][2] = invdet  * (m[0][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) + m[0][2] *
                                 (m[1][3] * m[3][1] - m[1][1] * m[3][3]) + m[0][3] * (m[1][1] * m[3][2] - m[1][2] * m[3][1]));
        res.m[0][3] = -invdet * (m[0][1] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) + m[0][2] *
                                 (m[1][3] * m[2][1] - m[1][1] * m[2][3]) + m[0][3] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]));
        res.m[1][0] = -invdet * (m[1][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) + m[1][2] *
                                 (m[2][3] * m[3][0] - m[2][0] * m[3][3]) + m[1][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]));
        res.m[1][1] = invdet  * (m[0][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) + m[0][2] *
                                 (m[2][3] * m[3][0] - m[2][0] * m[3][3]) + m[0][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]));
        res.m[1][2] = -invdet * (m[0][0] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) + m[0][2] *
                                 (m[1][3] * m[3][0] - m[1][0] * m[3][3]) + m[0][3] * (m[1][0] * m[3][2] - m[1][2] * m[3][0]));
        res.m[1][3] = invdet  * (m[0][0] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) + m[0][2] *
                                 (m[1][3] * m[2][0] - m[1][0] * m[2][3]) + m[0][3] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]));
        res.m[2][0] = invdet  * (m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) + m[1][1] *
                                 (m[2][3] * m[3][0] - m[2][0] * m[3][3]) + m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));
        res.m[2][1] = -invdet * (m[0][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) + m[0][1] *
                                 (m[2][3] * m[3][0] - m[2][0] * m[3][3]) + m[0][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));
        res.m[2][2] = invdet  * (m[0][0] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) + m[0][1] *
                                 (m[1][3] * m[3][0] - m[1][0] * m[3][3]) + m[0][3] * (m[1][0] * m[3][1] - m[1][1] * m[3][0]));
        res.m[2][3] = -invdet * (m[0][0] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]) + m[0][1] *
                                 (m[1][3] * m[2][0] - m[1][0] * m[2][3]) + m[0][3] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]));
        res.m[3][0] = -invdet * (m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) + m[1][1] *
                                 (m[2][2] * m[3][0] - m[2][0] * m[3][2]) + m[1][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));
        res.m[3][1] = invdet  * (m[0][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) + m[0][1] *
                                 (m[2][2] * m[3][0] - m[2][0] * m[3][2]) + m[0][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));
        res.m[3][2] = -invdet * (m[0][0] * (m[1][1] * m[3][2] - m[1][2] * m[3][1]) + m[0][1] *
                                 (m[1][2] * m[3][0] - m[1][0] * m[3][2]) + m[0][2] * (m[1][0] * m[3][1] - m[1][1] * m[3][0]));
        res.m[3][3] = invdet  * (m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) + m[0][1] *
                                 (m[1][2] * m[2][0] - m[1][0] * m[2][2]) + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]));
        return res;
}


void Matrix4f::CalcClipPlanes(Vector4f& l, Vector4f& r, Vector4f& b, Vector4f& t, Vector4f& n, Vector4f& f) const
{
    Vector4f Row1(m[0][0], m[0][1], m[0][2], m[0][3]);
    Vector4f Row2(m[1][0], m[1][1], m[1][2], m[1][3]);
    Vector4f Row3(m[2][0], m[2][1], m[2][2], m[2][3]);
    Vector4f Row4(m[3][0], m[3][1], m[3][2], m[3][3]);

    l = Row1 + Row4;
    r = Row1 - Row4;
    b = Row2 + Row4;
    t = Row2 - Row4;
    n = Row3 + Row4;
    f = Row3 - Row4;
}

Quaternion::Quaternion(float Angle, const Vector3f& V)
{
    float HalfAngleInRadians = ToRadian(Angle/2);

    float SineHalfAngle = sinf(HalfAngleInRadians);
    float CosHalfAngle = cosf(HalfAngleInRadians);

    x = V.x * SineHalfAngle;
    y = V.y * SineHalfAngle;
    z = V.z * SineHalfAngle;
    w = CosHalfAngle;
}

Quaternion::Quaternion(float _x, float _y, float _z, float _w)
{
    x = _x;
    y = _y;
    z = _z;
    w = _w;
}

void Quaternion::Normalize()
{
    float Length = sqrtf(x * x + y * y + z * z + w * w);

    x /= Length;
    y /= Length;
    z /= Length;
    w /= Length;
}


Quaternion Quaternion::Conjugate() const
{
    Quaternion ret(-x, -y, -z, w);
    return ret;
}


Quaternion operator*(const Quaternion& q, const Vector3f& v)
{
    float w = - (q.x * v.x) - (q.y * v.y) - (q.z * v.z);
    float x =   (q.w * v.x) + (q.y * v.z) - (q.z * v.y);
    float y =   (q.w * v.y) + (q.z * v.x) - (q.x * v.z);
    float z =   (q.w * v.z) + (q.x * v.y) - (q.y * v.x);

    Quaternion ret(x, y, z, w);

    return ret;
}


Quaternion operator*(const Quaternion& l, const Quaternion& r)
{
    float w = (l.w * r.w) - (l.x * r.x) - (l.y * r.y) - (l.z * r.z);
    float x = (l.x * r.w) + (l.w * r.x) + (l.y * r.z) - (l.z * r.y);
    float y = (l.y * r.w) + (l.w * r.y) + (l.z * r.x) - (l.x * r.z);
    float z = (l.z * r.w) + (l.w * r.z) + (l.x * r.y) - (l.y * r.x);

    Quaternion ret(x, y, z, w);

    return ret;
}



Vector3f Quaternion::ToDegrees()
{
    float f[3];

    f[0] = atan2(x * z + y * w, x * w - y * z);
    f[1] = acos(-x * x - y * y - z * z - w * w);
    f[2] = atan2(x * z - y * w, x * w + y * z);

    f[0] = ToDegree(f[0]);
    f[1] = ToDegree(f[1]);
    f[2] = ToDegree(f[2]);

    return Vector3f(f);
}


float RandomFloat()
{
    float Max = RAND_MAX;
    return ((float)RANDOM() / Max);
}


float RandomFloatRange(float Start, float End)
{
    if (End == Start) {
        printf("Invalid random range: (%f, %f)\n", Start, End);
        exit(0);
    }

    float Delta = End - Start;

    float RandomValue = RandomFloat() * Delta + Start;

    return RandomValue;
}


void CalcTightLightProjection(const Matrix4f& CameraView,        // in
                              const Vector3f& LightDir,          // in
                              const PersProjInfo& persProjInfo,  // in
                              Vector3f& LightPosWorld,           // out
                              OrthoProjInfo& orthoProjInfo)      // out
{
    //
    // Step #1: Calculate the view frustum corners in view space
    //
    Frustum frustum;
    frustum.CalcCorners(persProjInfo);

    //
    // Step #2: transform the view frustum to world space
    //
    Matrix4f InverseCameraView = CameraView.Inverse();
    frustum.Transform(InverseCameraView);

    Frustum view_frustum_in_world_space = frustum;   // backup for later

    //
    // Step #3: Transform the view frustum to light space (1st time)
    //
    Matrix4f LightView;
    Vector3f Origin(0.0f, 0.0f, 0.0f);
    Vector3f Up(0.0f, 1.0f, 0.0f);
    LightView.InitCameraTransform(Origin, LightDir, Up);
    frustum.Transform(LightView);

    //
    // Step #4: Calculate an AABB for the view frustum in light space
    //
    AABB aabb;
    frustum.CalcAABB(aabb);

    //
    // Step #5: Calculate the position of the light
    //
    Vector3f BottomLeft(aabb.MinX, aabb.MinY, aabb.MinZ);
    Vector3f TopRight(aabb.MaxX, aabb.MaxY, aabb.MinZ);
    Vector4f LightPosWorld4d = Vector4f((BottomLeft + TopRight) / 2.0f, 1.0f);

    //
    // Step #6: transform the position of the light back to world space
    //
    Matrix4f LightViewInv = LightView.Inverse();
    LightPosWorld4d = LightViewInv * LightPosWorld4d;
    LightPosWorld = Vector3f(LightPosWorld4d.x, LightPosWorld4d.y, LightPosWorld4d.z);

    //
    // Step #7: transform the view frustum to light space (2nd time)
    //
    LightView.InitCameraTransform(LightPosWorld, LightDir, Up);
    view_frustum_in_world_space.Transform(LightView);

    //
    // Step #8: with the light in its final position recalculate the aabb
    //
    AABB final_aabb;
    view_frustum_in_world_space.CalcAABB(final_aabb);
    final_aabb.UpdateOrthoInfo(orthoProjInfo);
}


int CalcNextPowerOfTwo(int x)
{
    int ret = 1;

    if (x == 1) {
        return 2;
    }

    while (ret < x) {
        ret = ret * 2;
    }

    return ret;
}


bool IsPointInsideViewFrustum(const Vector3f& p, const Matrix4f& VP)
{
    Vector4f p4D(p, 1.0f);

    Vector4f ClipSpaceP = VP * p4D;

    bool InsideViewFrustum = ((ClipSpaceP.x <=  ClipSpaceP.w) &&
                              (ClipSpaceP.x >= -ClipSpaceP.w) &&
                              (ClipSpaceP.y <=  ClipSpaceP.w) &&
                              (ClipSpaceP.y >= -ClipSpaceP.w) &&
                              (ClipSpaceP.z <=  ClipSpaceP.w) &&
                              (ClipSpaceP.z >= -ClipSpaceP.w));

    return InsideViewFrustum;
}
