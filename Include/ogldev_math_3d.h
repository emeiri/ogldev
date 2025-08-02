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

#ifndef MATH_3D_H
#define MATH_3D_H

#ifdef _WIN64
#ifndef _USE_MATH_DEFINES
// For some reason this works on in Debug build of Visual Studio but not in Release build.
// The solution people suggested was to define this as a preprocessor macro in the project.
#define _USE_MATH_DEFINES 
#endif
#include <cmath>
#else
#include <math.h>
#endif
#include <stdio.h>
#include <cfloat>
#include <numbers>

#include "ogldev_util.h"

#include <assimp/vector3.h>
#include <assimp/matrix3x3.h>
#include <assimp/matrix4x4.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

// powf wrapper for integer params to avoid crazy casting
#define powi(base,exp) (int)powf((float)(base), (float)(exp))

#define ToRadian(x) (float)(((x) * std::numbers::pi / 180.0f))
#define ToDegree(x) (float)(((x) * 180.0f / std::numbers::pi))

static inline float RandomFloat()
{
    float Max = RAND_MAX;
    return ((float)RANDOM() / Max);
}


float RandomFloatRange(float Start, float End);

struct Vector2i
{
    Vector2i() {}

    Vector2i(int xi, int yi)
    {
        x = xi;
        y = yi;
    }

    int x = 0;
    int y = 0;
};

struct Vector3i
{
    union {
        float x;
        float r;
    };

    union {
        float y;
        float g;
    };

    union {
        float z;
        float b;
    };
};

struct Vector2f
{
    union {
        float x = 0.0f;
        float u;
    };

    union {
        float y = 0.0f;
        float v;
    };

    Vector2f()
    {
    }

    Vector2f(float f)
    {
        x = f;
        y = f;
    }

    Vector2f(float _x, float _y)
    {
        x = _x;
        y = _y;
    }

    void Print(bool endl = true) const
    {
        printf("(%f, %f)", x, y);

        if (endl) {
            printf("\n");
        }
    }

    float Length() const
    {
        float len = sqrtf(x * x + y * y);
        return len;
    }


    void Normalize()
    {
        float len = Length();

        assert(len != 0);

        x /= len;
        y /= len;
    }
};


inline Vector2f operator*(const Vector2f& l, float f)
{
    Vector2f Ret(l.x * f, l.y * f);

    return Ret;
}


struct Vector4f;

struct Vector3f
{
    union {
        float x = 0.0f;
        float r;
    };

    union {
        float y = 0.0f;
        float g;
    };

    union {
        float z = 0.0f;
        float b;
    };

    Vector3f() {}

    Vector3f(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    Vector3f(const float* pFloat)
    {
        x = pFloat[0];
        y = pFloat[1];
        z = pFloat[2];
    }


    Vector3f(const glm::vec3& v)
    {
        x = v[0];
        y = v[1];
        z = v[2];
    }

    void InitRandom(const Vector3f& MinVal, const Vector3f& MaxVal);

    void InitBySphericalCoords(float Radius, float Pitch, float Heading)
    {
        x = Radius * cosf(ToRadian(Pitch)) * sinf(ToRadian(Heading));
        y = -Radius * sinf(ToRadian(Pitch));
        z = Radius * cosf(ToRadian(Pitch)) * cosf(ToRadian(Heading));
    }

    Vector3f(float f)
    {
        x = y = z = f;
    }

    Vector3f(const Vector4f& v);

    Vector3f& operator+=(const Vector3f& r)
    {
        x += r.x;
        y += r.y;
        z += r.z;

        return *this;
    }

    Vector3f& operator-=(const Vector3f& r)
    {
        x -= r.x;
        y -= r.y;
        z -= r.z;

        return *this;
    }

    Vector3f& operator*=(float f)
    {
        x *= f;
        y *= f;
        z *= f;

        return *this;
    }

    bool operator==(const Vector3f& r)
    {
        return ((x == r.x) && (y == r.y) && (z == r.z));
    }

    bool operator!=(const Vector3f& r)
    {
        return !(*this == r);
    }

    operator const float*() const
    {
        return &(x);
    }

    Vector3f Cross(const Vector3f& v) const;

    float Dot(const Vector3f& v) const
    {
        float ret = x * v.x + y * v.y + z * v.z;
        return ret;
    }

    float Distance(const Vector3f& v) const
    {
        float delta_x = x - v.x;
        float delta_y = y - v.y;
        float delta_z = z - v.z;
        float distance = sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);
        return distance;
    }

    float Length() const
    {
        float len = sqrtf(x * x + y * y + z * z);
        return len;
    }

    bool IsZero() const
    {
        return ((x + y + z) == 0.0f);
    }

    Vector3f& Normalize();

    void Rotate(float Angle, const Vector3f& Axis);

    Vector3f Negate() const;

    void Print(bool endl = true) const
    {
        printf("(%f, %f, %f)", x, y, z);

        if (endl) {
            printf("\n");
        }
    }

    float* data()
    {
        return &x;
    }

    void SetAll(float f)
    {
        x = y = z = 0.0f;
    }

    void SetZero()
    {
        SetAll(0.0f);
    }


    glm::vec3 ToGLM() const
    {
        glm::vec3 ret(x, y, z);
        return ret;
    }
};


struct Vector4f
{
    union {
        float x = 0.0f;
        float r;
    };

    union {
        float y = 0.0f;
        float g;
    };

    union {
        float z = 0.0f;
        float b;
    };

    union {
        float w = 0.0f;
        float a;
    };

    Vector4f()
    {
    }

    Vector4f(float _x, float _y, float _z, float _w)
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    Vector4f(const Vector3f& v, float _w)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        w = _w;
    }

    Vector4f(float f)
    {
        x = y = z = w = f;
    }

    void Print(bool endl = true) const
    {
        printf("(%f, %f, %f, %f)", x, y, z, w);

        if (endl) {
            printf("\n");
        }
    }

    Vector3f to3D() const
    {
        Vector3f v(x, y, z);
        return v;
    }

    float Length() const
    {
        float len = sqrtf(x * x + y * y + z * z + w * w);
        return len;
    }

    Vector4f& Normalize();

    float Dot(const Vector4f& v) const
    {
        float ret = x * v.x + y * v.y + z * v.z + w * v.w;
        return ret;
    }

    bool operator==(const Vector4f& r)
    {
        return ((x == r.x) && (y == r.y) && (z == r.z) && (w == r.w));
    }

    bool operator!=(const Vector4f& r)
    {
        return !(*this == r);
    }

    const float* data() const
    {
        const float* p = &x;
        return p;
    }
};



inline Vector3f operator+(const Vector3f& l, const Vector3f& r)
{
    Vector3f Ret(l.x + r.x,
                 l.y + r.y,
                 l.z + r.z);

    return Ret;
}

inline Vector3f operator-(const Vector3f& l, const Vector3f& r)
{
    Vector3f Ret(l.x - r.x,
                 l.y - r.y,
                 l.z - r.z);

    return Ret;
}

inline Vector3f operator*(const Vector3f& l, float f)
{
    Vector3f Ret(l.x * f,
                 l.y * f,
                 l.z * f);

    return Ret;
}


inline Vector3f operator/(const Vector3f& l, float f)
{
    Vector3f Ret(l.x / f,
                 l.y / f,
                 l.z / f);

    return Ret;
}


inline Vector3f::Vector3f(const Vector4f& v)
{
    x = v.x;
    y = v.y;
    z = v.z;
}


inline Vector4f operator+(const Vector4f& l, const Vector4f& r)
{
    Vector4f Ret(l.x + r.x,
                 l.y + r.y,
                 l.z + r.z,
                 l.w + r.w);

    return Ret;
}


inline Vector4f operator-(const Vector4f& l, const Vector4f& r)
{
    Vector4f Ret(l.x - r.x,
                 l.y - r.y,
                 l.z - r.z,
                 l.w - r.w);

    return Ret;
}


inline Vector4f operator/(const Vector4f& l, float f)
{
    Vector4f Ret(l.x / f,
                 l.y / f,
                 l.z / f,
                 l.w / f);

    return Ret;
}


inline Vector4f operator*(const Vector4f& l, float f)
{
    Vector4f Ret(l.x * f,
                 l.y * f,
                 l.z * f,
                 l.w * f);

    return Ret;
}


inline Vector4f operator*(float f, const Vector4f& l)
{
    Vector4f Ret(l.x * f,
                 l.y * f,
                 l.z * f,
                 l.w * f);

    return Ret;
}


struct PersProjInfo
{
    float FOV = 0.0f;
    float Width = 0.0f;
    float Height = 0.0f;
    float zNear = 0.0f;
    float zFar = 0.0f;
};


struct OrthoProjInfo
{
    float r;        // right
    float l;        // left
    float b;        // bottom
    float t;        // top
    float n;        // z near
    float f;        // z far

    float Width;
    float Height;    

    void Print()
    {
        printf("Left %f   Right %f\n", l, r);
        printf("Bottom %f Top %f\n", b, t);
        printf("Near %f   Far %f\n", n, f);
    }
};


struct Quaternion
{
    float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

    Quaternion() {}

    Quaternion(float Angle, const Vector3f& V);

    Quaternion(float _x, float _y, float _z, float _w);

    void Normalize();

    Quaternion Conjugate() const;

    Vector3f ToDegrees();

    bool IsZero() const;
};

Quaternion operator*(const Quaternion& l, const Quaternion& r);

Quaternion operator*(const Quaternion& q, const Vector3f& v);

class Matrix3f;

class Matrix4f
{
public:
    float m[4][4] = { 0.0f };

    Matrix4f()  {}

    Matrix4f(float a00, float a01, float a02, float a03,
             float a10, float a11, float a12, float a13,
             float a20, float a21, float a22, float a23,
             float a30, float a31, float a32, float a33)
    {
        m[0][0] = a00; m[0][1] = a01; m[0][2] = a02; m[0][3] = a03;
        m[1][0] = a10; m[1][1] = a11; m[1][2] = a12; m[1][3] = a13;
        m[2][0] = a20; m[2][1] = a21; m[2][2] = a22; m[2][3] = a23;
        m[3][0] = a30; m[3][1] = a31; m[3][2] = a32; m[3][3] = a33;
    }

    // constructor from Assimp matrix
    Matrix4f(const aiMatrix4x4& AssimpMatrix)
    {
        m[0][0] = AssimpMatrix.a1; m[0][1] = AssimpMatrix.a2; m[0][2] = AssimpMatrix.a3; m[0][3] = AssimpMatrix.a4;
        m[1][0] = AssimpMatrix.b1; m[1][1] = AssimpMatrix.b2; m[1][2] = AssimpMatrix.b3; m[1][3] = AssimpMatrix.b4;
        m[2][0] = AssimpMatrix.c1; m[2][1] = AssimpMatrix.c2; m[2][2] = AssimpMatrix.c3; m[2][3] = AssimpMatrix.c4;
        m[3][0] = AssimpMatrix.d1; m[3][1] = AssimpMatrix.d2; m[3][2] = AssimpMatrix.d3; m[3][3] = AssimpMatrix.d4;
    }

    Matrix4f(const aiMatrix3x3& AssimpMatrix)
    {
        m[0][0] = AssimpMatrix.a1; m[0][1] = AssimpMatrix.a2; m[0][2] = AssimpMatrix.a3; m[0][3] = 0.0f;
        m[1][0] = AssimpMatrix.b1; m[1][1] = AssimpMatrix.b2; m[1][2] = AssimpMatrix.b3; m[1][3] = 0.0f;
        m[2][0] = AssimpMatrix.c1; m[2][1] = AssimpMatrix.c2; m[2][2] = AssimpMatrix.c3; m[2][3] = 0.0f;
        m[3][0] = 0.0f           ; m[3][1] = 0.0f           ; m[3][2] = 0.0f           ; m[3][3] = 1.0f;
    }


    Matrix4f(const glm::mat4& a)
    {
        m[0][0] = a[0][0]; m[0][1] = a[1][0]; m[0][2] = a[2][0]; m[0][3] = a[3][0];
        m[1][0] = a[0][1]; m[1][1] = a[1][1]; m[1][2] = a[2][1]; m[1][3] = a[3][1];
        m[2][0] = a[0][2]; m[2][1] = a[1][2]; m[2][2] = a[2][2]; m[2][3] = a[3][2];
        m[3][0] = a[0][3]; m[3][1] = a[1][3]; m[3][2] = a[2][3]; m[3][3] = a[3][3];
    }

    Matrix4f(const Matrix3f& a);

    void SetZero()
    {
        ZERO_MEM(m);
    }

    Matrix4f Transpose() const
    {
        Matrix4f n;

        for (unsigned int i = 0 ; i < 4 ; i++) {
            for (unsigned int j = 0 ; j < 4 ; j++) {
                n.m[i][j] = m[j][i];
            }
        }

        return n;
    }


    inline void InitIdentity()
    {
        m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
        m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
        m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;
        m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
    }

    inline Matrix4f operator*(const Matrix4f& Right) const
    {
        Matrix4f Ret;

        for (unsigned int i = 0 ; i < 4 ; i++) {
            for (unsigned int j = 0 ; j < 4 ; j++) {
                Ret.m[i][j] = m[i][0] * Right.m[0][j] +
                              m[i][1] * Right.m[1][j] +
                              m[i][2] * Right.m[2][j] +
                              m[i][3] * Right.m[3][j];
            }
        }

        return Ret;
    }

    Vector4f operator*(const Vector4f& v) const
    {
        Vector4f r;

        r.x = m[0][0]* v.x + m[0][1]* v.y + m[0][2]* v.z + m[0][3]* v.w;
        r.y = m[1][0]* v.x + m[1][1]* v.y + m[1][2]* v.z + m[1][3]* v.w;
        r.z = m[2][0]* v.x + m[2][1]* v.y + m[2][2]* v.z + m[2][3]* v.w;
        r.w = m[3][0]* v.x + m[3][1]* v.y + m[3][2]* v.z + m[3][3]* v.w;

        return r;
    }

    operator const float*() const
    {
        return &(m[0][0]);
    }


    const float* data() const
    {
        return &(m[0][0]);
    }


    void Print() const
    {
        for (int i = 0 ; i < 4 ; i++) {
            printf("%f %f %f %f\n", m[i][0], m[i][1], m[i][2], m[i][3]);
        }
    }

    float Determinant() const;

    Matrix4f Inverse() const;

    void InitScaleTransform(float ScaleX, float ScaleY, float ScaleZ);
    void InitScaleTransform(float Scale);
    void InitScaleTransform(const Vector3f& Scale);

    void InitRotateTransform(float RotateX, float RotateY, float RotateZ);
    void InitRotateTransformZYX(float RotateX, float RotateY, float RotateZ);
    void InitRotateTransform(const Vector3f& Rotate);
    void InitRotateTransform(const Quaternion& quat);
    void InitRotateTransform(const glm::quat& quat);
    void InitRotationFromDir(const Vector3f& Dir);

    void InitTranslationTransform(float x, float y, float z);
    void InitTranslationTransform(const Vector3f& Pos);

    void InitCameraTransform(const Vector3f& Target, const Vector3f& Up);

    void InitCameraTransform(const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up);

    void InitPersProjTransform(const PersProjInfo& p);

    void InitOrthoProjTransform(const OrthoProjInfo& p);

    void CalcClipPlanes(Vector4f& l, Vector4f& r, Vector4f& b, Vector4f& t, Vector4f& n, Vector4f& f) const;

private:
    void InitRotationX(float RotateX);
    void InitRotationY(float RotateY);
    void InitRotationZ(float RotateZ);
};


class Matrix3f
{
public:
    float m[3][3] = { 0.0f };

    Matrix3f()  {}

    // Initialize the matrix from the top left corner of the 4-by-4 matrix
    Matrix3f(const Matrix4f& a)
    {
        m[0][0] = a.m[0][0]; m[0][1] = a.m[0][1]; m[0][2] = a.m[0][2];
        m[1][0] = a.m[1][0]; m[1][1] = a.m[1][1]; m[1][2] = a.m[1][2];
        m[2][0] = a.m[2][0]; m[2][1] = a.m[2][1]; m[2][2] = a.m[2][2];
    }

    Vector3f operator*(const Vector3f& v) const
    {
        Vector3f r;

        r.x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z;
        r.y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z;
        r.z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z;

        return r;
    }

    inline Matrix3f operator*(const Matrix3f& Right) const
    {
        Matrix3f Ret;

        for (unsigned int i = 0 ; i < 3 ; i++) {
            for (unsigned int j = 0 ; j < 3 ; j++) {
                Ret.m[i][j] = m[i][0] * Right.m[0][j] +
                              m[i][1] * Right.m[1][j] +
                              m[i][2] * Right.m[2][j];
            }
        }

        return Ret;
    }

    Matrix3f Transpose() const
    {
        Matrix3f n;

        for (unsigned int i = 0 ; i < 3 ; i++) {
            for (unsigned int j = 0 ; j < 3 ; j++) {
                n.m[i][j] = m[j][i];
            }
        }

        return n;
    }

    void InitRotateTransform(float RotateX, float RotateY, float RotateZ);

    void Print() const
    {
        for (int i = 0 ; i < 3 ; i++) {
            printf("%f %f %f\n", m[i][0], m[i][1], m[i][2]);
        }
    }

private:
    void InitRotationX(float RotateX);
    void InitRotationY(float RotateY);
    void InitRotationZ(float RotateZ);
};


class AABB
{
public:
    AABB() {}

    void Add(const Vector3f& v)
    {
        MinX = min(MinX, v.x);
        MinY = min(MinY, v.y);
        MinZ = min(MinZ, v.z);

        MaxX = max(MaxX, v.x);
        MaxY = max(MaxY, v.y);
        MaxZ = max(MaxZ, v.z);
    }

    float MinX = FLT_MAX;
    float MaxX = FLT_MIN;
    float MinY = FLT_MAX;
    float MaxY = FLT_MIN;
    float MinZ = FLT_MAX;
    float MaxZ = FLT_MIN;

    void Print()
    {
        printf("X: [%f,%f]\n", MinX, MaxX);
        printf("Y: [%f,%f]\n", MinY, MaxY);
        printf("Z: [%f,%f]\n", MinZ, MaxZ);
    }

    void UpdateOrthoInfo(struct OrthoProjInfo& o)
    {
        o.r = MaxX;
        o.l = MinX;
        o.b = MinY;
        o.t = MaxY;
        o.n = MinZ;
        o.f = MaxZ;
    }
};


class Frustum
{
public:
    Vector4f NearTopLeft;
    Vector4f NearBottomLeft;
    Vector4f NearTopRight;
    Vector4f NearBottomRight;

    Vector4f FarTopLeft;
    Vector4f FarBottomLeft;
    Vector4f FarTopRight;
    Vector4f FarBottomRight;

    Frustum() {}

    void CalcCorners(const PersProjInfo& persProjInfo)
    {
        float AR = persProjInfo.Height / persProjInfo.Width;

        float tanHalfFOV = tanf(ToRadian(persProjInfo.FOV / 2.0f));

        float NearZ = persProjInfo.zNear;
        float NearX = NearZ * tanHalfFOV;
        float NearY = NearZ * tanHalfFOV * AR;

        NearTopLeft     = Vector4f(-NearX, NearY, NearZ, 1.0f);
        NearBottomLeft  = Vector4f(-NearX, -NearY, NearZ, 1.0f);
        NearTopRight    = Vector4f(NearX, NearY, NearZ, 1.0f);
        NearBottomRight = Vector4f(NearX, -NearY, NearZ, 1.0f);

        float FarZ = persProjInfo.zFar;
        float FarX = FarZ * tanHalfFOV;
        float FarY = FarZ * tanHalfFOV * AR;

        FarTopLeft     = Vector4f(-FarX, FarY, FarZ, 1.0f);
        FarBottomLeft  = Vector4f(-FarX, -FarY, FarZ, 1.0f);
        FarTopRight    = Vector4f(FarX, FarY, FarZ, 1.0f);
        FarBottomRight = Vector4f(FarX, -FarY, FarZ, 1.0f);
    }


    void Transform(const Matrix4f& m)
    {
         NearTopLeft     = m * NearTopLeft;
         NearBottomLeft  = m * NearBottomLeft;
         NearTopRight    = m * NearTopRight;
         NearBottomRight = m * NearBottomRight;

         FarTopLeft     = m * FarTopLeft;
         FarBottomLeft  = m * FarBottomLeft;
         FarTopRight    = m * FarTopRight;
         FarBottomRight = m * FarBottomRight;
    }


    void CalcAABB(AABB& aabb)
    {
        aabb.Add(NearTopLeft);
        aabb.Add(NearBottomLeft);
        aabb.Add(NearTopRight);
        aabb.Add(NearBottomRight);

        aabb.Add(FarTopLeft);
        aabb.Add(FarBottomLeft);
        aabb.Add(FarTopRight);
        aabb.Add(FarBottomRight);
    }


    void Print()
    {
        printf("NearTopLeft "); NearTopLeft.Print();
        printf("NearBottomLeft "); NearBottomLeft.Print();
        printf("NearTopRight "); NearTopRight.Print();
        printf("NearBottomLeft "); NearBottomRight.Print();

        printf("FarTopLeft "); FarTopLeft.Print();
        printf("FarBottomLeft "); FarBottomLeft.Print();
        printf("FarTopRight "); FarTopRight.Print();
        printf("FarBottomLeft "); FarBottomRight.Print();
    }
};


class FrustumCulling
{
public:

    FrustumCulling(const Matrix4f& ViewProj)
    {
        Update(ViewProj);
    }

    void Update(const Matrix4f& ViewProj)
    {
        ViewProj.CalcClipPlanes(m_leftClipPlane,
                                m_rightClipPlane,
                                m_bottomClipPlane,
                                m_topClipPlane,
                                m_nearClipPlane,
                                m_farClipPlane);
    }

    bool IsPointInsideViewFrustum(const Vector3f& p) const
    {
        Vector4f p4D(p, 1.0f);

        bool Inside =
            (m_leftClipPlane.Dot(p4D)   >= 0) &&
            (m_rightClipPlane.Dot(p4D)  <= 0) &&
         //   (m_topClipPlane.Dot(p4D)    <= 0) &&
         //   (m_bottomClipPlane.Dot(p4D) >= 0) &&
            (m_nearClipPlane.Dot(p4D)   >= 0) &&
            (m_farClipPlane.Dot(p4D)    <= 0);

        return Inside;
    }

private:

    Vector4f m_leftClipPlane;
    Vector4f m_rightClipPlane;
    Vector4f m_bottomClipPlane;
    Vector4f m_topClipPlane;
    Vector4f m_nearClipPlane;
    Vector4f m_farClipPlane;
};

void CalcTightLightProjection(const Matrix4f& CameraView,        // in
                              const Vector3f& LightDir,          // in
                              const PersProjInfo& persProjInfo,  // in
                              Vector3f& LightPosWorld,           // out
                              OrthoProjInfo& orthoProjInfo);     // out

int CalcNextPowerOfTwo(int x);

bool IsPointInsideViewFrustum(const Vector3f& p, const Matrix4f& VP);

glm::quat RotationBetweenVectors(glm::vec3& start, glm::vec3& dest);

#define GLM_PRINT_VEC3(s, v) printf("%s (%f,%f,%f)\n", s, v.x, v.y, v.z)

#endif  /* MATH_3D_H */
