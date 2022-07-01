#include <stdio.h>
#include <cfloat>

#include "../Common/math_3d.cpp"


//
// Configuration params
//
Vector3f CameraPos(1.0f, 0.0f, 5.0f);
Vector3f CameraTarget(0.0f, 0.0f, 1.0f);
Vector3f LightDir(0.0f, 0.0f, 1.0f);


class AABB {
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

int main(int argc, char* argv[])
{
    float NearZ = 1.0f;
    float FarZ = 100.0f;
    float FOV = 90.0f;

    float WindowHeight = 1000;
    float WindowWidth = 2000;

    float AR = WindowHeight / WindowWidth;

    float tanHalfFOV = tanf(ToRadian(FOV / 2.0f));

    printf("TanHalfFov %f\n", tanHalfFOV);

    float NearX = NearZ * tanHalfFOV;
    float NearY = NearZ * tanHalfFOV * AR;

    //
    // Step #1: Calculate frustum corners in view space
    //

    Vector4f NearTopLeft(-NearX, NearY, NearZ, 1.0f);
    Vector4f NearBottomLeft(-NearX, -NearY, NearZ, 1.0f);
    Vector4f NearTopRight(NearX, NearY, NearZ, 1.0f);
    Vector4f NearBottomRight(NearX, -NearY, NearZ, 1.0f);

    printf("Frustum in view space\n\n");
    printf("NearTopLeft "); NearTopLeft.Print();
    printf("NearBottomLeft "); NearBottomLeft.Print();
    printf("NearTopRight "); NearTopRight.Print();
    printf("NearBottomLeft "); NearBottomRight.Print();

    float FarX = FarZ * tanHalfFOV;
    float FarY = FarZ * tanHalfFOV * AR;

    Vector4f FarTopLeft(-FarX, FarY, FarZ, 1.0f);
    Vector4f FarBottomLeft(-FarX, -FarY, FarZ, 1.0f);
    Vector4f FarTopRight(FarX, FarY, FarZ, 1.0f);
    Vector4f FarBottomRight(FarX, -FarY, FarZ, 1.0f);

    printf("FarTopLeft "); FarTopLeft.Print();
    printf("FarBottomLeft "); FarBottomLeft.Print();
    printf("FarTopRight "); FarTopRight.Print();
    printf("FarBottomLeft "); FarBottomRight.Print();
    printf("\n");

    //
    // Step #2: transform frustum to world space
    //

    Matrix4f View;

    Vector3f Up(0.0f, 1.0f, 0.0f);
    View.InitCameraTransform(CameraPos, CameraTarget, Up);
    printf("View transformations:\n");
    View.Print();
    printf("\n");

    //    Matrix3f View3f(View);  // Initialize using the top left corner
    Matrix4f InverseView = View.Inverse();

    printf("Inverse view transformation\n");
    InverseView.Print();
    printf("\n");

    Vector4f NearTopLeftWorld     = InverseView * NearTopLeft;
    Vector4f NearBottomLeftWorld  = InverseView * NearBottomLeft;
    Vector4f NearTopRightWorld    = InverseView * NearTopRight;
    Vector4f NearBottomRightWorld = InverseView * NearBottomRight;

    Vector3f LightPosWorld = (Vector3f(NearBottomLeftWorld) + Vector3f(NearTopRightWorld)) / 2.0f;
    printf("LightPos: "); LightPosWorld.Print(); printf("\n");

    Vector4f FarTopLeftWorld     = InverseView * FarTopLeft;
    Vector4f FarBottomLeftWorld  = InverseView * FarBottomLeft;
    Vector4f FarTopRightWorld    = InverseView * FarTopRight;
    Vector4f FarBottomRightWorld = InverseView * FarBottomRight;

    printf("Frustum in world space\n\n");

    printf("NearTopLeft "); NearTopLeftWorld.Print();
    printf("NearBottomLeft "); NearBottomLeftWorld.Print();
    printf("NearTopRight "); NearTopRightWorld.Print();
    printf("NearBottomLeft "); NearBottomRightWorld.Print();

    printf("FarTopLeft "); FarTopLeftWorld.Print();
    printf("FarBottomLeft "); FarBottomLeftWorld.Print();
    printf("FarTopRight "); FarTopRightWorld.Print();
    printf("FarBottomLeft "); FarBottomRightWorld.Print();
    printf("\n");
    //
    // Step #3: Transform frustum to light space
    //

    Matrix4f LightView;
    LightView.InitCameraTransform(LightPosWorld, LightDir, Up);

    Vector4f NearTopLeftLight     = LightView * NearTopLeftWorld;
    Vector4f NearBottomLeftLight  = LightView * NearBottomLeftWorld;
    Vector4f NearTopRightLight    = LightView * NearTopRightWorld;
    Vector4f NearBottomRightLight = LightView * NearBottomRightWorld;

    Vector4f FarTopLeftLight     = LightView * FarTopLeftWorld;
    Vector4f FarBottomLeftLight  = LightView * FarBottomLeftWorld;
    Vector4f FarTopRightLight    = LightView * FarTopRightWorld;
    Vector4f FarBottomRightLight = LightView * FarBottomRightWorld;

    printf("Frustum in light space\n\n");

    printf("NearTopLeft "); NearTopLeftLight.Print();
    printf("NearBottomLeft "); NearBottomLeftLight.Print();
    printf("NearTopRight "); NearTopRightLight.Print();
    printf("NearBottomLeft "); NearBottomRightLight.Print();

    printf("FarTopLeft "); FarTopLeftLight.Print();
    printf("FarBottomLeft "); FarBottomLeftLight.Print();
    printf("FarTopRight "); FarTopRightLight.Print();
    printf("FarBottomLeft "); FarBottomRightLight.Print();
    printf("\n");

    //
    // Step #4: Calculate an AABB
    //

    AABB aabb;
    aabb.Add(NearTopLeftLight);
    aabb.Add(NearBottomLeftLight);
    aabb.Add(NearTopRightLight);
    aabb.Add(NearBottomRightLight);

    aabb.Add(FarTopLeftLight);
    aabb.Add(FarBottomLeftLight);
    aabb.Add(FarTopRightLight);
    aabb.Add(FarBottomRightLight);

    printf("AABB\n");
    aabb.Print();
    return 0;
}
