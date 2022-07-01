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

    Matrix4f View;
    Vector3f Up(0.0f, 1.0f, 0.0f);
    View.InitCameraTransform(CameraPos, CameraTarget, Up);
    printf("View transformations:\n"); View.Print();printf("\n");

    Vector3f LightPosWorld;
    OrthoProjInfo orthoProjInfo;
    CalcTightLightProjection(View, LightDir, persProjInfo, LightPosWorld, orthoProjInfo);
    printf("\nFinal orthographic projection params\n");
    orthoProjInfo.Print();
    return 0;
}
