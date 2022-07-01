#include <stdio.h>
#include <cfloat>

#include "../Common/math_3d.cpp"


//
// Configuration params
//
Vector3f CameraPos(1.0f, 0.0f, 5.0f);
Vector3f CameraTarget(0.0f, 0.0f, 1.0f);
Vector3f LightDir(0.0f, 0.0f, 1.0f);

// Perpsective projection info
float NearZ = 1.0f;
float FarZ = 100.0f;
float FOV = 90.0f;
float WindowHeight = 1000;
float WindowWidth = 2000;




int main(int argc, char* argv[])
{
    PersProjInfo persProjInfo;
    persProjInfo.FOV = FOV;
    persProjInfo.Width = WindowWidth;
    persProjInfo.Height = WindowHeight;
    persProjInfo.zNear = NearZ;
    persProjInfo.zFar = FarZ;

    //
    // Step #1: Calculate frustum corners in view space
    //
    Frustum frustum;
    frustum.Calculate(persProjInfo);

    printf("Frustum in view space\n\n"); frustum.Print(); printf("\n");

    //
    // Step #2: transform frustum to world space
    //

    Matrix4f View;
    Vector3f Up(0.0f, 1.0f, 0.0f);
    View.InitCameraTransform(CameraPos, CameraTarget, Up);
    printf("View transformations:\n"); View.Print();printf("\n");

    Matrix4f InverseView = View.Inverse();
    printf("Inverse view transformation\n"); InverseView.Print(); printf("\n");

    frustum.Transform(InverseView);
    printf("Frustum in world space\n\n"); frustum.Print(); printf("\n");

    Vector3f LightPosWorld = (Vector3f(frustum.NearBottomLeft) + Vector3f(frustum.NearTopRight)) / 2.0f;
    printf("LightPos: "); LightPosWorld.Print(); printf("\n");

    //
    // Step #3: Transform frustum to light space
    //

    Matrix4f LightView;
    LightView.InitCameraTransform(LightPosWorld, LightDir, Up);
    frustum.Transform(LightView);
    printf("Frustum in light space\n\n"); frustum.Print(); printf("\n");

    //
    // Step #4: Calculate an AABB
    //

    AABB aabb;
    frustum.CalcAABB(aabb);

    printf("AABB\n");
    aabb.Print();
    return 0;
}
