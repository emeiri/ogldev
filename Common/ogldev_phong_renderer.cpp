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
#include "ogldev_phong_renderer.h"


PhongRenderer::PhongRenderer()
{

}


PhongRenderer::~PhongRenderer()
{

}


void PhongRenderer::InitPhongRenderer()
{
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
    //    m_skinningTech.SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);

    if (!m_shadowMapTech.Init()) {
        printf("Error initializing the shadow mapping technique\n");
        exit(1);
    }

    glUseProgram(0);
}


void PhongRenderer::StartShadowPass()
{
    m_shadowMapTech.Enable();
}


void PhongRenderer::SwitchToLightingTech()
{
    GLint cur_prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &cur_prog);

    if (cur_prog != m_lightingTech.GetProgram()) {
        m_lightingTech.Enable();
    }
}


void PhongRenderer::SwitchToSkinningTech()
{
    GLint cur_prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &cur_prog);

    if (cur_prog != m_skinningTech.GetProgram()) {
        m_skinningTech.Enable();
    }
}


void PhongRenderer::SetDirLight(const DirectionalLight& DirLight)
{
    m_dirLight = DirLight;

    SwitchToLightingTech();
    m_lightingTech.SetDirectionalLight(m_dirLight, false);

    m_skinningTech.Enable();
    m_skinningTech.SetDirectionalLight(m_dirLight, false);
}


void PhongRenderer::SetPointLights(uint NumLights, const PointLight* pPointLights)
{
    if (!pPointLights || (NumLights == 0)) {
        m_numPointLights = 0;
        return;
    }

    if (NumLights > LightingTechnique::MAX_POINT_LIGHTS) {
        printf("Number of point lights (%d) exceeds max (%d)\n", NumLights, LightingTechnique::MAX_POINT_LIGHTS);
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


void PhongRenderer::SetSpotLights(uint NumLights, const SpotLight* pSpotLights)
{
    if (!pSpotLights || (NumLights == 0)) {
        m_numSpotLights = 0;
        return;
    }

    if (NumLights > LightingTechnique::MAX_SPOT_LIGHTS) {
        printf("Number of point lights (%d) exceeds max (%d)\n", NumLights, LightingTechnique::MAX_SPOT_LIGHTS);
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


void PhongRenderer::UpdateDirLightDir(const Vector3f& WorldDir)
{
    m_dirLight.WorldDirection = WorldDir;
}


void PhongRenderer::UpdatePointLightPos(uint Index, const Vector3f& WorldPos)
{
    if (Index > m_numPointLights) {
        printf("Trying to update point light %d while num lights is %d\n", Index, m_numPointLights);
        exit(0);
    }

    m_pointLights[Index].WorldPosition = WorldPos;
}


void PhongRenderer::UpdateSpotLightPosAndDir(uint Index, const Vector3f& WorldPos, const Vector3f& WorldDir)
{
    if (Index > m_numSpotLights) {
        printf("Trying to update spot light %d while num lights is %d\n", Index, m_numSpotLights);
        exit(0);
    }

    m_spotLights[Index].WorldPosition = WorldPos;
    m_spotLights[Index].WorldDirection = WorldDir;

}


void PhongRenderer::Render(BasicMesh* pMesh)
{
    if (!m_pCamera) {
        printf("PhongRenderer: camera not initialized\n");
        exit(0);
    }

    if ((m_numPointLights == 0) && (m_numSpotLights == 0) && m_dirLight.IsZero()) {
        printf("Warning! trying to render but all lights are zero\n");
    }

    SwitchToLightingTech();

    Matrix4f WVP;
    GetWVP(pMesh, WVP);
    m_lightingTech.SetWVP(WVP);

    RefreshLightingPosAndDirs(pMesh);

    if (m_dirLight.DiffuseIntensity > 0.0) {
        m_lightingTech.SetDirectionalLight(m_dirLight);
    }

    m_lightingTech.UpdatePointLightsPos(m_numPointLights, m_pointLights);

    m_lightingTech.UpdateSpotLightsPosAndDir(m_numSpotLights, m_spotLights);

    m_lightingTech.SetMaterial(pMesh->GetMaterial());

    if (m_isPBR) {
        m_lightingTech.SetPBR(true);
        m_lightingTech.SetPBRMaterial(pMesh->GetPBRMaterial());
    } else {
        m_lightingTech.SetPBR(false);
    }

    Vector3f CameraLocalPos3f = pMesh->GetWorldTransform().WorldPosToLocalPos(m_pCamera->GetPos());
    m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);

    m_lightingTech.SetCameraWorldPos(m_pCamera->GetPos());

    Matrix4f World = pMesh->GetWorldTransform().GetMatrix();
    m_lightingTech.SetWorldMatrix(World);

    pMesh->Render();
}


void PhongRenderer::RenderAnimation(SkinnedMesh* pMesh, float AnimationTimeSec, int AnimationIndex)
{
    RenderAnimationCommon(pMesh);

    vector<Matrix4f> Transforms;
    pMesh->GetBoneTransforms(AnimationTimeSec, Transforms, AnimationIndex);

    for (uint i = 0 ; i < Transforms.size() ; i++) {
        m_skinningTech.SetBoneTransform(i, Transforms[i]);
    }

    pMesh->Render();
}


void PhongRenderer::RenderAnimationBlended(SkinnedMesh* pMesh,
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

    pMesh->Render();
}


void PhongRenderer::RenderAnimationCommon(SkinnedMesh* pMesh)
{
    if (!m_pCamera) {
        printf("PhongRenderer: camera not initialized\n");
        exit(0);
    }

    if ((m_numPointLights == 0) && (m_numSpotLights == 0) && m_dirLight.IsZero()) {
        printf("Warning! trying to render but all lights are zero\n");
    }

    SwitchToSkinningTech();

    Matrix4f WVP;
    GetWVP(pMesh, WVP);
    m_skinningTech.SetWVP(WVP);

    RefreshLightingPosAndDirs(pMesh);

    if (m_dirLight.DiffuseIntensity > 0.0) {
        m_skinningTech.UpdateDirLightDirection(m_dirLight);
    }

    m_skinningTech.UpdatePointLightsPos(m_numPointLights, m_pointLights);

    m_skinningTech.UpdateSpotLightsPosAndDir(m_numSpotLights, m_spotLights);

    m_skinningTech.SetMaterial(pMesh->GetMaterial());

    PBRMaterial Material;
    Material.Roughness = 0.43f;
    Material.IsMetal = false;
    Material.Color = Vector3f(1.0f, 0.71f, 0.29f); //pMesh->GetMaterial().DiffuseColor;
    m_skinningTech.SetPBRMaterial(Material);

    Vector3f CameraLocalPos3f = pMesh->GetWorldTransform().WorldPosToLocalPos(m_pCamera->GetPos());
    m_skinningTech.SetCameraLocalPos(CameraLocalPos3f);

    m_skinningTech.SetCameraWorldPos(m_pCamera->GetPos());

    Matrix4f World = pMesh->GetWorldTransform().GetMatrix();
    m_skinningTech.SetWorldMatrix(World);

}

void PhongRenderer::RenderToShadowMap(BasicMesh* pMesh, const SpotLight& SpotLight)
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


void PhongRenderer::RefreshLightingPosAndDirs(BasicMesh* pMesh)
{
    WorldTrans& meshWorldTransform = pMesh->GetWorldTransform();

    if (m_dirLight.DiffuseIntensity > 0.0) {
        m_dirLight.CalcLocalDirection(meshWorldTransform);
        //        m_dirLight.GetLocalDirection().Print();
    }

    for (uint i = 0 ; i < m_numPointLights ; i++) {
        m_pointLights[i].CalcLocalPosition(meshWorldTransform);
    }

    for (uint i = 0 ; i < m_numSpotLights ; i++) {
        m_spotLights[i].CalcLocalDirectionAndPosition(meshWorldTransform);
    }
}


void PhongRenderer::GetWVP(BasicMesh* pMesh, Matrix4f& WVP)
{
    WorldTrans& meshWorldTransform = pMesh->GetWorldTransform();

    Matrix4f World = meshWorldTransform.GetMatrix();
    Matrix4f View = m_pCamera->GetMatrix();
    Matrix4f Projection = m_pCamera->GetProjectionMat();

    WVP = Projection * View * World;
}


void PhongRenderer::ControlRimLight(bool IsEnabled)
{
    SwitchToLightingTech();
    m_lightingTech.ControlRimLight(IsEnabled);

    m_skinningTech.Enable();
    m_skinningTech.ControlRimLight(IsEnabled);
}


void PhongRenderer::ControlCellShading(bool IsEnabled)
{
    SwitchToLightingTech();
    m_lightingTech.ControlCellShading(IsEnabled);

    m_skinningTech.Enable();
    m_skinningTech.ControlCellShading(IsEnabled);
}


void PhongRenderer::SetLinearFog(float FogStart, float FogEnd, const Vector3f& FogColor)
{
    SwitchToLightingTech();
    m_lightingTech.SetLinearFog(FogStart, FogEnd);
    m_lightingTech.SetFogColor(FogColor);

    SwitchToSkinningTech();
    m_skinningTech.SetLinearFog(FogStart, FogEnd);
    m_skinningTech.SetFogColor(FogColor);
}


void PhongRenderer::SetExpFog(float FogEnd, const Vector3f& FogColor, float FogDensity)
{
    SwitchToLightingTech();
    m_lightingTech.SetExpFog(FogEnd, FogDensity);
    m_lightingTech.SetFogColor(FogColor);

    SwitchToSkinningTech();
    m_skinningTech.SetExpFog(FogEnd, FogDensity);
    m_skinningTech.SetFogColor(FogColor);
}


void PhongRenderer::SetExpSquaredFog(float FogEnd, const Vector3f& FogColor, float FogDensity)
{
    SwitchToLightingTech();
    m_lightingTech.SetExpSquaredFog(FogEnd, FogDensity);
    m_lightingTech.SetFogColor(FogColor);

    SwitchToSkinningTech();
    m_skinningTech.SetExpSquaredFog(FogEnd, FogDensity);
    m_skinningTech.SetFogColor(FogColor);
}


void PhongRenderer::SetLayeredFog(float FogTop, float FogEnd, const Vector3f& FogColor)
{
    SwitchToLightingTech();
    m_lightingTech.SetLayeredFog(FogTop, FogEnd);
    m_lightingTech.SetFogColor(FogColor);

    SwitchToSkinningTech();
    m_skinningTech.SetLayeredFog(FogTop, FogEnd);
    m_skinningTech.SetFogColor(FogColor);
}


void PhongRenderer::SetAnimatedFog(float FogEnd, float FogDensity, const Vector3f& FogColor)
{
    SwitchToLightingTech();
    m_lightingTech.SetAnimatedFog(FogEnd, FogDensity);
    m_lightingTech.SetFogColor(FogColor);

    SwitchToSkinningTech();
    m_skinningTech.SetAnimatedFog(FogEnd, FogDensity);
    m_skinningTech.SetFogColor(FogColor);
}


void PhongRenderer::UpdateAnimatedFogTime(float FogTime)
{
    SwitchToLightingTech();
    m_lightingTech.SetFogTime(FogTime);

    SwitchToSkinningTech();
    m_skinningTech.SetFogTime(FogTime);
}

void PhongRenderer::DisableFog()
{
    SwitchToLightingTech();
    m_lightingTech.SetFogColor(Vector3f(0.0f, 0.0f, 0.0f));

    SwitchToSkinningTech();
    m_skinningTech.SetFogColor(Vector3f(0.0f, 0.0f, 0.0f));
}


void PhongRenderer::SetPBR(bool IsPBR)
{
    m_isPBR = IsPBR;

    SwitchToLightingTech();
    m_lightingTech.SetPBR(IsPBR);

    SwitchToSkinningTech();
    m_skinningTech.SetPBR(IsPBR);

}
