#include <stdio.h>

#include "../Common/math_3d.cpp"

int main(int argc, char* argv[])
{
    float NearZ = 0.1f;
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

    Vector3f NearTopLeft(-NearX, NearY, NearZ);
    Vector3f NearBottomLeft(-NearX, -NearY, NearZ);
    Vector3f NearTopRight(NearX, NearY, NearZ);
    Vector3f NearBottomRight(NearX, -NearY, NearZ);

    printf("Frustum in view space\n\n");
    printf("NearTopLeft "); NearTopLeft.Print();
    printf("NearBottomLeft "); NearBottomLeft.Print();
    printf("NearTopRight "); NearTopRight.Print();
    printf("NearBottomLeft "); NearBottomRight.Print();

    float FarX = FarZ * tanHalfFOV;
    float FarY = FarZ * tanHalfFOV * AR;

    Vector3f FarTopLeft(-FarX, FarY, FarZ);
    Vector3f FarBottomLeft(-FarX, -FarY, FarZ);
    Vector3f FarTopRight(FarX, FarY, FarZ);
    Vector3f FarBottomRight(FarX, -FarY, FarZ);

    printf("FarTopLeft "); FarTopLeft.Print();
    printf("FarBottomLeft "); FarBottomLeft.Print();
    printf("FarTopRight "); FarTopRight.Print();
    printf("FarBottomLeft "); FarBottomRight.Print();
    printf("\n");

    //
    // Step #2: transform frustum to world space
    //

    Matrix4f View;
    Vector3f Target(0.0f, 0.0f, 1.0f);
    Vector3f Up(0.0f, 1.0f, 0.0f);
    View.InitCameraTransform(Target, Up);
    printf("View transformations:\n");
    View.Print();
    printf("\n");
    Matrix3f View3f(View);  // Initialize using the top left corner

    // Inverse local-to-world transformation using transpose
    // (assuming uniform scaling)
    Matrix3f InverseView = View3f.Transpose();

    printf("Inverse view transformation\n");
    InverseView.Print();
    printf("\n");
    //    sleep(3);

    Vector3f NearTopLeftWorld     = InverseView * NearTopLeft;
    Vector3f NearBottomLeftWorld  = InverseView * NearBottomLeft;
    Vector3f NearTopRightWorld    = InverseView * NearTopRight;
    Vector3f NearBottomRightWorld = InverseView * NearBottomRight;

    Vector3f FarTopLeftWorld     = InverseView * FarTopLeft;
    Vector3f FarBottomLeftWorld  = InverseView * FarBottomLeft;
    Vector3f FarTopRightWorld    = InverseView * FarTopRight;
    Vector3f FarBottomRightWorld = InverseView * FarBottomRight;

    printf("Frustum in world space\n\n");

    printf("NearTopLeft "); NearTopLeftWorld.Print();
    printf("NearBottomLeft "); NearBottomLeftWorld.Print();
    printf("NearTopRight "); NearTopRightWorld.Print();
    printf("NearBottomLeft "); NearBottomRightWorld.Print();

    printf("FarTopLeft "); FarTopLeftWorld.Print();
    printf("FarBottomLeft "); FarBottomLeftWorld.Print();
    printf("FarTopRight "); FarTopRightWorld.Print();
    printf("FarBottomLeft "); FarBottomRightWorld.Print();

    //
    // Step #3: Transform frustum to light space
    //

    Matrix4f LightView4f;
    Vector3f Origin(0.0f, 0.0f, 0.0f);
    Vector3f LightDir(-1.0f, 0.0f, 0.0f);
    LightView4f.InitCameraTransform(Origin, LightDir, Up);
    Matrix3f LightView(LightView4f);

    Vector3f NearTopLeftLight     = LightView * NearTopLeftWorld;
    Vector3f NearBottomLeftLight  = LightView * NearBottomLeftWorld;
    Vector3f NearTopRightLight    = LightView * NearTopRightWorld;
    Vector3f NearBottomRightLight = LightView * NearBottomRightWorld;

    Vector3f FarTopLeftLight     = LightView * FarTopLeftWorld;
    Vector3f FarBottomLeftLight  = LightView * FarBottomLeftWorld;
    Vector3f FarTopRightLight    = LightView * FarTopRightWorld;
    Vector3f FarBottomRightLight = LightView * FarBottomRightWorld;

    printf("Frustum in light space\n\n");

    printf("NearTopLeft "); NearTopLeftLight.Print();
    printf("NearBottomLeft "); NearBottomLeftLight.Print();
    printf("NearTopRight "); NearTopRightLight.Print();
    printf("NearBottomLeft "); NearBottomRightLight.Print();

    printf("FarTopLeft "); FarTopLeftLight.Print();
    printf("FarBottomLeft "); FarBottomLeftLight.Print();
    printf("FarTopRight "); FarTopRightLight.Print();
    printf("FarBottomLeft "); FarBottomRightLight.Print();

    return 0;
}
