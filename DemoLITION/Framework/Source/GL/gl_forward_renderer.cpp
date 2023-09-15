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
#include "GL/gl_forward_renderer.h"
#include "GL/gl_rendering_system.h"

#define SHADOW_MAP_WIDTH 2048
#define SHADOW_MAP_HEIGHT 2048



ForwardRenderer::ForwardRenderer()
{

}


ForwardRenderer::~ForwardRenderer()
{
}


void ForwardRenderer::InitForwardRenderer(RenderingSystemGL* pRenderingSystemGL)
{
    if (!pRenderingSystemGL) {
        printf("%s:%d - must provide a rendering system\n", __FILE__, __LINE__);
        exit(1);
    }

    m_pRenderingSystemGL = pRenderingSystemGL;

    InitTechniques();

    InitShadowMapping();

    glUseProgram(0);
}


void ForwardRenderer::InitTechniques()
{
    if (!m_lightingTech.Init()) {
        printf("Error initializing the lighting technique\n");
        exit(1);
    }

    m_lightingTech.Enable();
    m_lightingTech.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetShadowMapTextureUnit(SHADOW_TEXTURE_UNIT_INDEX);

    //    m_lightingTech.SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);

    if (!m_shadowMapTech.Init()) {
        printf("Error initializing the shadow mapping technique\n");
        exit(1);
    }

    if (!m_flatColorTech.Init()) {
        printf("Error initializing the flat color technique\n");
        exit(1);
    }
}


void ForwardRenderer::InitShadowMapping()
{
    float FOV = 45.0f; // TODO: get from light
    float zNear = 1.0f;
    float zFar = 100.0f;
    PersProjInfo shadowPersProjInfo = { FOV, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, zNear, zFar };
    m_lightPersProjMatrix.InitPersProjTransform(shadowPersProjInfo);

    if (!m_shadowMapFBO.Init(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT)) {
        exit(1);
    }
}


void ForwardRenderer::SwitchToLightingTech()
{
    GLint cur_prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &cur_prog);

    if (cur_prog != m_lightingTech.GetProgram()) {
        m_lightingTech.Enable();
    }
}


void ForwardRenderer::Render(GLScene* pScene)
{
    if (!m_pCurCamera) {
        printf("ForwardRenderer: camera not initialized\n");
        exit(0);
    }

    if (pScene->GetRenderList().size() == 0) {
        printf("Warning! render list is empty and no main model\n");
        return;
    }

    if (pScene->IsClearFrame()) {
        const Vector4f& ClearColor = pScene->GetClearColor();
        glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    ShadowMapPass(pScene);
    LightingPass(pScene);

    m_curRenderPass = RENDER_PASS_UNINITIALIZED;
}


void ForwardRenderer::ShadowMapPass(GLScene* pScene)
{
    m_curRenderPass = RENDER_PASS_SHADOW;

    m_shadowMapFBO.BindForWriting();

    glClear(GL_DEPTH_BUFFER_BIT);

    m_shadowMapTech.Enable();

    const std::list<CoreSceneObject*>& RenderList = pScene->GetRenderList();
    std::list<CoreSceneObject*>::const_iterator it = RenderList.begin();
    CoreSceneObject* pSceneObject = *it;

    Vector3f Up(0.0f, 0.0f, 1.0f);

    int NumSpotLights = (int)pScene->m_spotLights.size();

    if (NumSpotLights > 0) {
        printf("fooo\n");
        exit(1);
    }
    else {
        const std::vector<SpotLight>& SpotLights = pSceneObject->GetModel()->GetSpotLights();

        if (SpotLights.size() > 0) {
            m_lightViewMatrix.InitCameraTransform(SpotLights[0].WorldPosition, SpotLights[0].WorldDirection, Up);
        }
    }

    for (std::list<CoreSceneObject*>::const_iterator it = RenderList.begin(); it != RenderList.end(); it++) {
        CoreSceneObject* pSceneObject = *it;
        pSceneObject->GetModel()->Render(this);
    }
}


void ForwardRenderer::LightingPass(GLScene* pScene)
{
    m_curRenderPass = RENDER_PASS_LIGHTING;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_shadowMapFBO.BindForReading(SHADOW_TEXTURE_UNIT);

    int WindowWidth = 0;
    int WindowHeight = 0;
    m_pRenderingSystemGL->GetWindowSize(WindowWidth, WindowHeight);

    glViewport(0, 0, WindowWidth, WindowHeight);

    bool FirstTimeForwardLighting = true;

    const std::list<CoreSceneObject*>& RenderList = pScene->GetRenderList();

    for (std::list<CoreSceneObject*>::const_iterator it = RenderList.begin(); it != RenderList.end(); it++) {
        CoreSceneObject* pSceneObject = *it;

        const Vector4f& FlatColor = pSceneObject->GetFlatColor();

        if (FlatColor.x == -1.0f) {
            if (FirstTimeForwardLighting) {
                StartRenderWithForwardLighting(pScene, pSceneObject);
                FirstTimeForwardLighting = false;
            }
            RenderWithForwardLighting(pScene, pSceneObject);
        }
        else {
            RenderWithFlatColor(pScene, pSceneObject);
        }
    }
}


void ForwardRenderer::StartRenderWithForwardLighting(GLScene* pScene, CoreSceneObject* pSceneObject)
{
    SwitchToLightingTech();

    int NumLightsTotal = 0;

    int NumPointLights = (int)pScene->m_pointLights.size();

    if (NumPointLights > 0) {
        m_lightingTech.SetPointLights(NumPointLights, &pScene->m_pointLights[0], true);
        NumLightsTotal += NumPointLights;
    } else {
        const std::vector<PointLight>& PointLights = pSceneObject->GetModel()->GetPointLights();

        if (PointLights.size() > 0) {
            m_lightingTech.SetPointLights((unsigned int)PointLights.size(), &PointLights[0], true);
            NumLightsTotal += (int)PointLights.size();
        }
    }

    int NumSpotLights = (int)pScene->m_spotLights.size();

    if (NumSpotLights > 0) {
        m_lightingTech.SetSpotLights(NumSpotLights, &pScene->m_spotLights[0], true);
        NumLightsTotal += NumPointLights;
    } else {
        const std::vector<SpotLight>& SpotLights = pSceneObject->GetModel()->GetSpotLights();

        if (SpotLights.size() > 0) {
            m_lightingTech.SetSpotLights((unsigned int)SpotLights.size(), &SpotLights[0], true);
            NumLightsTotal += (int)SpotLights.size();
        }
    }

    int NumDirLights = (int)pScene->m_dirLights.size();

    if (NumDirLights > 0) {
        const DirectionalLight& DirLight = pScene->m_dirLights[0];
        m_lightingTech.SetDirectionalLight(DirLight, true);    
        NumLightsTotal += NumDirLights;
    } else {
        const std::vector<DirectionalLight>& DirLights = pSceneObject->GetModel()->GetDirLights();

        if (DirLights.size() > 0) {
            const DirectionalLight& DirLight = DirLights[0];
            m_lightingTech.SetDirectionalLight(DirLight, true);
            NumLightsTotal += (int)DirLights.size();
        }
    }

    if (NumLightsTotal == 0) {
        printf("Warning! trying to render but all lights are zero\n");
        exit(0);
    }    

    m_lightingTech.SetCameraWorldPos(m_pCurCamera->GetPos());
}


void ForwardRenderer::RenderWithForwardLighting(GLScene* pScene, CoreSceneObject* pSceneObject)
{
    pSceneObject->GetModel()->Render(this);
}


void ForwardRenderer::RenderWithFlatColor(GLScene* pScene, CoreSceneObject* pSceneObject)
{
    m_flatColorTech.Enable();
    m_flatColorTech.SetColor(pSceneObject->GetFlatColor());
    Matrix4f WVP;
    GetWVP(pSceneObject, WVP);
    m_flatColorTech.SetWVP(WVP);
    pSceneObject->GetModel()->Render();
}



/*void ForwardRenderer::RenderAnimation(SkinnedMesh* pMesh, float AnimationTimeSec, int AnimationIndex)
{
    RenderAnimationCommon(pMesh);

    vector<Matrix4f> Transforms;
    pMesh->GetBoneTransforms(AnimationTimeSec, Transforms, AnimationIndex);

    for (uint i = 0 ; i < Transforms.size() ; i++) {
        m_skinningTech.SetBoneTransform(i, Transforms[i]);
    }

    pMesh->Render(&m_skinningTech);
}*/


/*void ForwardRenderer::RenderAnimationBlended(SkinnedMesh* pMesh,
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
}*/


/*void ForwardRenderer::RenderAnimationCommon(SkinnedMesh* pMesh)
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
}*/

void ForwardRenderer::RenderToShadowMap(CoreSceneObject* pSceneObject, const SpotLight& SpotLight)
{
    Matrix4f World = pSceneObject->GetMatrix();

    printf("World\n");
    World.Print();

    Matrix4f View;
    Vector3f Up(0.0f, 0.0f, 1.0f);
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

    pSceneObject->GetModel()->Render();
}


void ForwardRenderer::GetWVP(CoreSceneObject* pSceneObject, Matrix4f& WVP)
{
    Matrix4f World = pSceneObject->GetMatrix();
    Matrix4f View = m_pCurCamera->GetMatrix();
    Matrix4f Projection = m_pCurCamera->GetProjectionMat();

   // Projection.Print();
 //   exit(0);

    WVP = Projection * View * World;
}


void ForwardRenderer::ControlRimLight(bool IsEnabled)
{
    SwitchToLightingTech();
    m_lightingTech.ControlRimLight(IsEnabled);
}


void ForwardRenderer::ControlCellShading(bool IsEnabled)
{
    SwitchToLightingTech();
    m_lightingTech.ControlCellShading(IsEnabled);
}


void ForwardRenderer::SetLinearFog(float FogStart, float FogEnd, const Vector3f& FogColor)
{
    SwitchToLightingTech();
    m_lightingTech.SetLinearFog(FogStart, FogEnd);
    m_lightingTech.SetFogColor(FogColor);
}


void ForwardRenderer::SetExpFog(float FogEnd, const Vector3f& FogColor, float FogDensity)
{
    SwitchToLightingTech();
    m_lightingTech.SetExpFog(FogEnd, FogDensity);
    m_lightingTech.SetFogColor(FogColor);
}


void ForwardRenderer::SetExpSquaredFog(float FogEnd, const Vector3f& FogColor, float FogDensity)
{
    SwitchToLightingTech();
    m_lightingTech.SetExpSquaredFog(FogEnd, FogDensity);
    m_lightingTech.SetFogColor(FogColor);
}


void ForwardRenderer::SetLayeredFog(float FogTop, float FogEnd, const Vector3f& FogColor)
{
    SwitchToLightingTech();
    m_lightingTech.SetLayeredFog(FogTop, FogEnd);
    m_lightingTech.SetFogColor(FogColor);
}


void ForwardRenderer::SetAnimatedFog(float FogEnd, float FogDensity, const Vector3f& FogColor)
{
    SwitchToLightingTech();
    m_lightingTech.SetAnimatedFog(FogEnd, FogDensity);
    m_lightingTech.SetFogColor(FogColor);
}


void ForwardRenderer::UpdateAnimatedFogTime(float FogTime)
{
    SwitchToLightingTech();
    m_lightingTech.SetFogTime(FogTime);
}

void ForwardRenderer::DisableFog()
{
    SwitchToLightingTech();
    m_lightingTech.SetFogColor(Vector3f(0.0f, 0.0f, 0.0f));
}


void ForwardRenderer::DrawStart_CB(uint DrawIndex)
{
    // TODO: picking technique update
}


void ForwardRenderer::ControlSpecularExponent_CB(bool IsEnabled)
{
    if (m_curRenderPass == RENDER_PASS_LIGHTING) {
        m_lightingTech.ControlSpecularExponent(IsEnabled);
    }
}


void ForwardRenderer::SetMaterial_CB(const Material& material)
{
    if (m_curRenderPass == RENDER_PASS_LIGHTING) {
        m_lightingTech.SetMaterial(material);
    }
}


void ForwardRenderer::DisableDiffuseTexture_CB()
{
    if (m_curRenderPass == RENDER_PASS_LIGHTING) {
        m_lightingTech.DisableDiffuseTexture();
    }
}


void ForwardRenderer::SetWorldMatrix_CB(const Matrix4f& World)
{
    switch (m_curRenderPass) {

    case RENDER_PASS_SHADOW:
        SetWorldMatrix_CB_ShadowPass(World);
        break;

    case RENDER_PASS_LIGHTING:
        SetWorldMatrix_CB_LightingPass(World);
        break;

    default:
        printf("%s:%d - Unknown render pass %d\n", __FILE__, __LINE__, m_curRenderPass);
        exit(1);
    }
}


void ForwardRenderer::SetWorldMatrix_CB_ShadowPass(const Matrix4f& World)
{
    Matrix4f WVP = m_lightPersProjMatrix * m_lightViewMatrix * World;
    m_shadowMapTech.SetWVP(WVP);

}


void ForwardRenderer::SetWorldMatrix_CB_LightingPass(const Matrix4f& World)
{
    m_lightingTech.SetWorldMatrix(World);

    Matrix4f View = m_pCurCamera->GetMatrix();
    Matrix4f Projection = m_pCurCamera->GetProjectionMat();
    Matrix4f WVP = Projection * View * World;
    m_lightingTech.SetWVP(WVP);

    Matrix4f LightWVP = m_lightPersProjMatrix * m_lightViewMatrix * World;
    m_lightingTech.SetLightWVP(LightWVP);

    Matrix4f InverseWorld = World.Inverse();
    Matrix3f World3x3(InverseWorld);
    Matrix3f WorldTranspose = World3x3.Transpose();

    m_lightingTech.SetNormalMatrix(WorldTranspose);
}