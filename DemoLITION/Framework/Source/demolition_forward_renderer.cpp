/*

        Copyright 2011 Etay Meiri

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

#include "ogldev_engine_common.h"
#include "demolition_forward_renderer.h"


ForwardRenderer::ForwardRenderer()
{

}


ForwardRenderer::~ForwardRenderer()
{
    SAFE_DELETE(m_pDefaultCamera);
}


void ForwardRenderer::InitForwardRenderer(BaseRenderingSubsystem* pRenderingSubsystem)
{
    if (!pRenderingSubsystem) {
        printf("%s:%d - must provide a rendering system\n", __FILE__, __LINE__);
        exit(1);
    }

    m_pRenderingSubsystem = pRenderingSubsystem;

    if (!m_lightingTech.Init()) {
        printf("Error initializing the lighting technique\n");
        exit(1);
    }

    m_lightingTech.Enable();
    m_lightingTech.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    //    m_lightingTech.SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);

    if (!m_skinningTech.Init()) {
        printf("Error initializing the skinning technique\n");
        exit(1);
    }

    m_skinningTech.Enable();
    m_skinningTech.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    //    // m_skinningTech.SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);

    if (!m_shadowMapTech.Init()) {
        printf("Error initializing the shadow mapping technique\n");
        exit(1);
    }

    CreateDefaultCamera();

    glUseProgram(0);
}


void ForwardRenderer::CreateDefaultCamera()
{
    Vector3f Pos(0.0f, 0.0f, 0.0f);
    Vector3f Target(0.0f, 0.0f, 1.0f);
    Vector3f Up(0.0, 1.0f, 0.0f);

    float FOV = 45.0f;
    float zNear = 0.1f;
    float zFar = 100.0f;
    int WindowWidth = 0;
    int WindowHeight = 0;
    m_pRenderingSubsystem->GetWindowSize(WindowWidth, WindowHeight);

    PersProjInfo persProjInfo = { FOV, (float)WindowWidth, (float)WindowHeight, zNear, zFar };

    m_pDefaultCamera = new BasicCamera(persProjInfo, Pos, Target, Up);
    m_pCurCamera = m_pDefaultCamera;

    m_pRenderingSubsystem->SetCamera(m_pDefaultCamera);    
}


void ForwardRenderer::StartShadowPass()
{
    m_shadowMapTech.Enable();
}


void ForwardRenderer::SwitchToLightingTech()
{
    GLint cur_prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &cur_prog);

    if (cur_prog != m_lightingTech.GetProgram()) {
        m_lightingTech.Enable();
    }
}


void ForwardRenderer::SwitchToSkinningTech()
{
    GLint cur_prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &cur_prog);

    if (cur_prog != m_skinningTech.GetProgram()) {
        m_skinningTech.Enable();
    }
}


void ForwardRenderer::SetDirLight(const DirectionalLight& DirLight)
{
    m_dirLight = DirLight;

    SwitchToLightingTech();
    m_lightingTech.SetDirectionalLight(m_dirLight, true);

  //  m_skinningTech.Enable();
  //  m_skinningTech.SetDirectionalLight(m_dirLight, true);
}


void ForwardRenderer::SetPointLights(uint NumLights, const PointLight* pPointLights)
{
    if (!pPointLights || (NumLights == 0)) {
        m_numPointLights = 0;
        return;
    }

    if (NumLights > ForwardLightingTechnique::MAX_POINT_LIGHTS) {
        printf("Number of point lights (%d) exceeds max (%d)\n", NumLights, ForwardLightingTechnique::MAX_POINT_LIGHTS);
        exit(0);
    }

    for (uint i = 0 ; i < NumLights ; i++) {
        m_pointLights[i] = pPointLights[i];
    }

    m_numPointLights = NumLights;

    SwitchToLightingTech();
    m_lightingTech.SetPointLights(NumLights, pPointLights, false);

    m_skinningTech.Enable();
    m_skinningTech.SetPointLights(NumLights, pPointLights, false);
}


void ForwardRenderer::SetSpotLights(uint NumLights, const SpotLight* pSpotLights)
{
    if (!pSpotLights || (NumLights == 0)) {
        m_numSpotLights = 0;
        return;
    }

    if (NumLights > ForwardLightingTechnique::MAX_SPOT_LIGHTS) {
        printf("Number of point lights (%d) exceeds max (%d)\n", NumLights, ForwardLightingTechnique::MAX_SPOT_LIGHTS);
        exit(0);
    }

    for (uint i = 0 ; i < NumLights ; i++) {
        m_spotLights[i] = pSpotLights[i];
    }

    m_numSpotLights = NumLights;

    SwitchToLightingTech();
    m_lightingTech.SetSpotLights(NumLights, pSpotLights, false);

    m_skinningTech.Enable();
    m_skinningTech.SetSpotLights(NumLights, pSpotLights, false);
}


void ForwardRenderer::UpdateDirLightDir(const Vector3f& WorldDir)
{
    m_dirLight.WorldDirection = WorldDir;
}


void ForwardRenderer::UpdatePointLightPos(uint Index, const Vector3f& WorldPos)
{
    if (Index > m_numPointLights) {
        printf("Trying to update point light %d while num lights is %d\n", Index, m_numPointLights);
        exit(0);
    }

    m_pointLights[Index].WorldPosition = WorldPos;
}


void ForwardRenderer::UpdateSpotLightPosAndDir(uint Index, const Vector3f& WorldPos, const Vector3f& WorldDir)
{
    if (Index > m_numSpotLights) {
        printf("Trying to update spot light %d while num lights is %d\n", Index, m_numSpotLights);
        exit(0);
    }

    m_spotLights[Index].WorldPosition = WorldPos;
    m_spotLights[Index].WorldDirection = WorldDir;

}


void ForwardRenderer::Render(BasicMesh* pMesh)
{
    if (!m_pCurCamera) {
        printf("ForwardRenderer: camera not initialized\n");
        exit(0);
    }

    if ((m_numPointLights == 0) && (m_numSpotLights == 0) && m_dirLight.IsZero()) {
        printf("Warning! trying to render but all lights are zero\n");
    }

    SwitchToLightingTech();

    if (m_dirLight.DiffuseIntensity > 0.0) {
        m_lightingTech.UpdateDirLightDirection(m_dirLight);
    }

    m_lightingTech.UpdatePointLightsPos(m_numPointLights, m_pointLights);

    m_lightingTech.UpdateSpotLightsPosAndDir(m_numSpotLights, m_spotLights);

    m_lightingTech.SetMaterial(pMesh->GetMaterial());

    m_lightingTech.SetCameraWorldPos(m_pCurCamera->GetPos());

    UpdateMatrices(&m_lightingTech, pMesh);

    pMesh->Render(&m_lightingTech);
}


void ForwardRenderer::Render(GLScene* pScene)
{
    if (!m_pCurCamera) {
        printf("ForwardRenderer: camera not initialized\n");
        exit(0);
    }

    int NumPointLights = (int)pScene->m_pointLights.size();
    int NumSpotLights = (int)pScene->m_spotLights.size();
    int NumDirLights = (int)pScene->m_dirLights.size();

    if ((NumPointLights == 0) && (NumSpotLights == 0) && (NumDirLights == 0)) {
        printf("Warning! trying to render but all lights are zero\n");
    }

    BasicMesh* pMesh = pScene->GetObjectList().front();

    SwitchToLightingTech();

    const DirectionalLight& dirLight = pScene->m_dirLights[0];

    if (dirLight.DiffuseIntensity > 0.0) {
        SetDirLight(dirLight);
        //m_lightingTech.UpdateDirLightDirection(dirLight);
    }

    m_lightingTech.UpdatePointLightsPos(m_numPointLights, m_pointLights);

    m_lightingTech.UpdateSpotLightsPosAndDir(m_numSpotLights, m_spotLights);

    m_lightingTech.SetMaterial(pMesh->GetMaterial());

    m_lightingTech.SetCameraWorldPos(m_pCurCamera->GetPos());

    UpdateMatrices(&m_lightingTech, pMesh);

    pMesh->Render(&m_lightingTech);
}

void ForwardRenderer::Render(const Scene0& Scene)
{
    if (!m_pCurCamera) {
        printf("ForwardRenderer: camera not initialized\n");
        exit(0);
    }

    if ((Scene.m_pointLights.size() == 0) && (Scene.m_spotLights.size() == 0) && 
        ((Scene.m_dirLight.size() == 0) || (Scene.m_dirLight[0].IsZero()))) {
        printf("Warning! trying to render but all lights are zero\n");
    }    

    if ((Scene.m_dirLight.size() > 0) && (Scene.m_dirLight[0].DiffuseIntensity > 0.0)) {
        SetDirLight(Scene.m_dirLight[0]);
        //m_lightingTech.UpdateDirLightDirection(Scene.m_dirLight[0]);
    }

    SwitchToLightingTech();

    if (Scene.m_pointLights.size() > 0) {
        m_lightingTech.UpdatePointLightsPos((unsigned int)Scene.m_pointLights.size(), &Scene.m_pointLights[0]);
    }

    if (Scene.m_spotLights.size() > 0) {
        m_lightingTech.UpdateSpotLightsPosAndDir((unsigned int)Scene.m_spotLights.size(), &Scene.m_spotLights[0]);
    }

    m_lightingTech.SetMaterial(Scene.m_pMesh->GetMaterial());

    m_lightingTech.SetCameraWorldPos(m_pCurCamera->GetPos());

    UpdateMatrices(&m_lightingTech, Scene.m_pMesh);

    Scene.m_pMesh->Render(&m_lightingTech);
}



void ForwardRenderer::UpdateMatrices(ForwardLightingTechnique* pBaseTech, BasicMesh* pMesh)
{
    Matrix4f WVP;
    GetWVP(pMesh, WVP);
    pBaseTech->SetWVP(WVP);

    Matrix4f World = pMesh->GetWorldTransform().GetMatrix();
    pBaseTech->SetWorldMatrix(World);

    Matrix4f InverseWorld = World.Inverse();
    Matrix3f World3x3(InverseWorld);
    Matrix3f WorldTranspose = World3x3.Transpose();

    pBaseTech->SetNormalMatrix(WorldTranspose);
}


void ForwardRenderer::RenderAnimation(SkinnedMesh* pMesh, float AnimationTimeSec, int AnimationIndex)
{
    RenderAnimationCommon(pMesh);

    vector<Matrix4f> Transforms;
    pMesh->GetBoneTransforms(AnimationTimeSec, Transforms, AnimationIndex);

    for (uint i = 0 ; i < Transforms.size() ; i++) {
        m_skinningTech.SetBoneTransform(i, Transforms[i]);
    }

    pMesh->Render(&m_skinningTech);
}


void ForwardRenderer::RenderAnimationBlended(SkinnedMesh* pMesh,
                                           float AnimationTimeSec,
                                           int StartAnimIndex,
                                           int EndAnimIndex,
                                           float BlendFactor)
{
    RenderAnimationCommon(pMesh);

    vector<Matrix4f> Transforms;
    pMesh->GetBoneTransformsBlended(AnimationTimeSec,
                                    Transforms,
                                    StartAnimIndex,
                                    EndAnimIndex,
                                    BlendFactor);

    for (uint i = 0 ; i < Transforms.size() ; i++) {
        m_skinningTech.SetBoneTransform(i, Transforms[i]);
    }

    pMesh->Render(&m_skinningTech);
}


void ForwardRenderer::RenderAnimationCommon(SkinnedMesh* pMesh)
{
    if (!m_pCurCamera) {
        printf("ForwardRenderer: camera not initialized\n");
        exit(0);
    }

    if ((m_numPointLights == 0) && (m_numSpotLights == 0) && m_dirLight.IsZero()) {
        printf("Warning! trying to render but all lights are zero\n");
    }

    SwitchToSkinningTech();

    if (m_dirLight.DiffuseIntensity > 0.0) {
        m_skinningTech.UpdateDirLightDirection(m_dirLight);
    }

    m_skinningTech.UpdatePointLightsPos(m_numPointLights, m_pointLights);

    m_skinningTech.UpdateSpotLightsPosAndDir(m_numSpotLights, m_spotLights);

    m_skinningTech.SetMaterial(pMesh->GetMaterial());

    UpdateMatrices(&m_skinningTech, pMesh);
}

void ForwardRenderer::RenderToShadowMap(BasicMesh* pMesh, const SpotLight& SpotLight)
{
    Matrix4f World = pMesh->GetWorldTransform().GetMatrix();

    printf("World\n");
    World.Print();

    Matrix4f View;
    Vector3f Up(0.0f, 1.0f, 0.0f);
    View.InitCameraTransform(SpotLight.WorldPosition, SpotLight.WorldDirection, Up);

    printf("View\n");
    View.Print();
    //    exit(0);
    float FOV = 45.0f;
    float zNear = 0.1f;
    float zFar = 100.0f;
    PersProjInfo persProjInfo = { FOV, 1000.0f, 1000.0f, zNear, zFar };
    Matrix4f Projection;
    Projection.InitPersProjTransform(persProjInfo);

    Matrix4f WVP = Projection * View * World;

    m_shadowMapTech.SetWVP(WVP);

    pMesh->Render();
}


void ForwardRenderer::GetWVP(BasicMesh* pMesh, Matrix4f& WVP)
{
    WorldTrans& meshWorldTransform = pMesh->GetWorldTransform();

    Matrix4f World = meshWorldTransform.GetMatrix();
    Matrix4f View = m_pCurCamera->GetMatrix();
    Matrix4f Projection = m_pCurCamera->GetProjectionMat();

    WVP = Projection * View * World;
}


void ForwardRenderer::ControlRimLight(bool IsEnabled)
{
    SwitchToLightingTech();
    m_lightingTech.ControlRimLight(IsEnabled);

    m_skinningTech.Enable();
    m_skinningTech.ControlRimLight(IsEnabled);
}


void ForwardRenderer::ControlCellShading(bool IsEnabled)
{
    SwitchToLightingTech();
    m_lightingTech.ControlCellShading(IsEnabled);

    m_skinningTech.Enable();
    m_skinningTech.ControlCellShading(IsEnabled);
}


void ForwardRenderer::SetLinearFog(float FogStart, float FogEnd, const Vector3f& FogColor)
{
    SwitchToLightingTech();
    m_lightingTech.SetLinearFog(FogStart, FogEnd);
    m_lightingTech.SetFogColor(FogColor);

    SwitchToSkinningTech();
    m_skinningTech.SetLinearFog(FogStart, FogEnd);
    m_skinningTech.SetFogColor(FogColor);
}


void ForwardRenderer::SetExpFog(float FogEnd, const Vector3f& FogColor, float FogDensity)
{
    SwitchToLightingTech();
    m_lightingTech.SetExpFog(FogEnd, FogDensity);
    m_lightingTech.SetFogColor(FogColor);

    SwitchToSkinningTech();
    m_skinningTech.SetExpFog(FogEnd, FogDensity);
    m_skinningTech.SetFogColor(FogColor);
}


void ForwardRenderer::SetExpSquaredFog(float FogEnd, const Vector3f& FogColor, float FogDensity)
{
    SwitchToLightingTech();
    m_lightingTech.SetExpSquaredFog(FogEnd, FogDensity);
    m_lightingTech.SetFogColor(FogColor);

    SwitchToSkinningTech();
    m_skinningTech.SetExpSquaredFog(FogEnd, FogDensity);
    m_skinningTech.SetFogColor(FogColor);
}


void ForwardRenderer::SetLayeredFog(float FogTop, float FogEnd, const Vector3f& FogColor)
{
    SwitchToLightingTech();
    m_lightingTech.SetLayeredFog(FogTop, FogEnd);
    m_lightingTech.SetFogColor(FogColor);

    SwitchToSkinningTech();
    m_skinningTech.SetLayeredFog(FogTop, FogEnd);
    m_skinningTech.SetFogColor(FogColor);
}


void ForwardRenderer::SetAnimatedFog(float FogEnd, float FogDensity, const Vector3f& FogColor)
{
    SwitchToLightingTech();
    m_lightingTech.SetAnimatedFog(FogEnd, FogDensity);
    m_lightingTech.SetFogColor(FogColor);

    SwitchToSkinningTech();
    m_skinningTech.SetAnimatedFog(FogEnd, FogDensity);
    m_skinningTech.SetFogColor(FogColor);
}


void ForwardRenderer::UpdateAnimatedFogTime(float FogTime)
{
    SwitchToLightingTech();
    m_lightingTech.SetFogTime(FogTime);

    SwitchToSkinningTech();
    m_skinningTech.SetFogTime(FogTime);
}

void ForwardRenderer::DisableFog()
{
    SwitchToLightingTech();
    m_lightingTech.SetFogColor(Vector3f(0.0f, 0.0f, 0.0f));

    SwitchToSkinningTech();
    m_skinningTech.SetFogColor(Vector3f(0.0f, 0.0f, 0.0f));
}
