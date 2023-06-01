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


#ifdef _WIN64
#include <Windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ogldev_basic_glfw_camera.h"


struct Plane
{
    Vector3f normal = { 0.f, 1.f, 0.f }; // unit vector
    float     distance = 0.f;        // Distance with origin

    Plane() = default;

    Plane(const Vector3f& p1, const Vector3f& norm)
    {
        normal = norm;
        normal.Normalize();
        distance = normal.Dot(p1);
    }

    float getSignedDistanceToPlane(const Vector3f& point) const
    {
        return normal.Dot(point) - distance;
    }
};


struct Frustum2
{
    Plane topFace;
    Plane bottomFace;

    Plane rightFace;
    Plane leftFace;

    Plane farFace;
    Plane nearFace;
};


Frustum2 createFrustumFromCamera(const BasicCamera& cam, float aspect, float fovY, float zNear, float zFar)
{
    Frustum2     frustum;
    float halfVSide = zFar * tanf(fovY * .5f);
    float halfHSide = halfVSide * aspect;
    Vector3f frontMultFar = cam.GetTarget() * zFar;

    frustum.nearFace = { cam.GetPos() + cam.GetTarget() * zNear, cam.GetTarget() };
    frustum.farFace = { cam.GetPos() + frontMultFar, cam.GetTarget() * -1.0f };
    Vector3f Right = cam.GetUp().Cross(cam.GetTarget());
    Vector3f t = frontMultFar - Right * halfHSide;
    frustum.rightFace = { cam.GetPos(), t.Cross(cam.GetUp()) };
    t = frontMultFar + Right * halfHSide;
    frustum.leftFace = { cam.GetPos(), cam.GetUp().Cross(t) };
    frustum.topFace = { cam.GetPos(), Right.Cross(frontMultFar - cam.GetUp() * halfVSide) };
    t = frontMultFar + cam.GetUp() * halfVSide;
    frustum.bottomFace = { cam.GetPos(), t.Cross(Right) };

    return frustum;
}


static void BasicClipSpaceTest()
{
    Vector3f Pos(0.0f, 0.0f, 0.0f);
    Vector3f Target(0.0f, 0.0f, 1.0f);
    Vector3f Up(0.0, 1.0f, 0.0f);

    float FOV = 45.0f;
    float zNear = 1.0f;
    float zFar = 100.0f;
    float WindowWidth = 1000.0f;
    float WindowHeight = 1000.0f;
    PersProjInfo persProjInfo = { FOV, WindowWidth, WindowHeight, zNear, zFar };

    BasicCamera Camera(persProjInfo, Pos, Target, Up);

    Matrix4f VP = Camera.GetViewProjMatrix();

    Vector4f InsideFrustum1(0.0f, 0.0f, 5.0f, 1.0f);
    Vector4f InsideFrustum2(1.0f, 0.0f, 5.0f, 1.0f);
    Vector4f InsideFrustum3(-1.0f, 0.0f, 5.0f, 1.0f);
    Vector4f InsideFrustum4(0.0f, 1.0f, 5.0f, 1.0f);
    Vector4f InsideFrustum5(0.0f, -1.0f, 5.0f, 1.0f);
    Vector4f OutsideNearZ(0.0f, 0.0f, 0.5f, 1.0f);
    Vector4f OutsideFarZ(0.0f, 0.0f, 101.0f, 1.0f);
    Vector4f OutsideLeft(15.0f, 0.0f, 10.0f, 1.0f);
    Vector4f OutsideRight(-15.0f, 0.0f, 10.0f, 1.0f);
    Vector4f OutsideTop(0.0f, 15.0f, 10.0f, 1.0f);
    Vector4f OutsideBottom(0.0f, -15.0f, 10.0f, 1.0f);

    printf("Testing points inside the view frustum\n");
    Vector4f v;
    v = VP * InsideFrustum1; v.Print();
    v = VP * InsideFrustum2; v.Print();
    v = VP * InsideFrustum3; v.Print();
    v = VP * InsideFrustum4; v.Print();
    v = VP * InsideFrustum5; v.Print();

    printf("\nTesting outside near Z\n");
    v = VP * OutsideNearZ; v.Print();

    printf("\nTesting outside far Z\n");
    v = VP * OutsideFarZ; v.Print();

    printf("\nTesting outside left\n");
    v = VP * OutsideLeft; v.Print();

    printf("\nTesting outside right\n");
    v = VP * OutsideRight; v.Print();

    printf("\nTesting outside top\n");
    v = VP * OutsideTop; v.Print();

    printf("\nTesting outside bottom\n");
    v = VP * OutsideBottom; v.Print();
}


static void IsPointInsideClipSpaceTest()
{
    Vector3f Pos(0.0f, 0.0f, 0.0f);
    Vector3f Target(0.0f, 0.0f, 1.0f);
    Vector3f Up(0.0, 1.0f, 0.0f);

    float FOV = 45.0f;
    float zNear = 1.0f;
    float zFar = 100.0f;
    float WindowWidth = 1000.0f;
    float WindowHeight = 1000.0f;
    PersProjInfo persProjInfo = { FOV, WindowWidth, WindowHeight, zNear, zFar };

    BasicCamera Camera(persProjInfo, Pos, Target, Up);

    Matrix4f VP = Camera.GetViewProjMatrix();

    Vector4f InsideFrustum1(0.0f, 0.0f, 5.0f, 1.0f);
    Vector4f InsideFrustum2(1.0f, 0.0f, 5.0f, 1.0f);
    Vector4f InsideFrustum3(-1.0f, 0.0f, 5.0f, 1.0f);
    Vector4f InsideFrustum4(0.0f, 1.0f, 5.0f, 1.0f);
    Vector4f InsideFrustum5(0.0f, -1.0f, 5.0f, 1.0f);
    Vector4f OutsideNearZ(0.0f, 0.0f, 0.5f, 1.0f);
    Vector4f OutsideFarZ(0.0f, 0.0f, 101.0f, 1.0f);
    Vector4f OutsideLeft(15.0f, 0.0f, 10.0f, 1.0f);
    Vector4f OutsideRight(-15.0f, 0.0f, 10.0f, 1.0f);
    Vector4f OutsideTop(0.0f, 15.0f, 10.0f, 1.0f);
    Vector4f OutsideBottom(0.0f, -15.0f, 10.0f, 1.0f);

    printf("Testing points inside the view frustum\n");
    printf("Point1: %s\n", IsPointInsideViewFrustum(InsideFrustum1, VP) ? "Inside" : "Outside");
    printf("Point2: %s\n", IsPointInsideViewFrustum(InsideFrustum2, VP) ? "Inside" : "Outside");
    printf("Point3: %s\n", IsPointInsideViewFrustum(InsideFrustum3, VP) ? "Inside" : "Outside");
    printf("Point4: %s\n", IsPointInsideViewFrustum(InsideFrustum4, VP) ? "Inside" : "Outside");
    printf("Point5: %s\n", IsPointInsideViewFrustum(InsideFrustum5, VP) ? "Inside" : "Outside");

    printf("\nTesting outside near Z: %s\n", IsPointInsideViewFrustum(OutsideNearZ, VP) ? "Inside" : "Outside");
    
    printf("\nTesting outside far Z: %s\n", IsPointInsideViewFrustum(OutsideFarZ, VP) ? "Inside" : "Outside");

    printf("\nTesting outside left: %s\n", IsPointInsideViewFrustum(OutsideLeft, VP) ? "Inside" : "Outside");

    printf("\nTesting outside right: %s\n", IsPointInsideViewFrustum(OutsideRight, VP) ? "Inside" : "Outside");

    printf("\nTesting outside top: %s\n", IsPointInsideViewFrustum(OutsideTop, VP) ? "Inside" : "Outside");

    printf("\nTesting outside bottom: %s\n", IsPointInsideViewFrustum(OutsideBottom, VP) ? "Inside" : "Outside");
}

int main(int argc, char* argv[])
{
    //BasicClipSpaceTest();
    IsPointInsideClipSpaceTest();
    return 0;
}
    
