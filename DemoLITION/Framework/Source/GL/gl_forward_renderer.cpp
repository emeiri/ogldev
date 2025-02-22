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

extern bool UsePVP;
extern bool UseIndirectRender;

struct CameraDirection
{
    GLenum CubemapFace;
    Vector3f Target;
    Vector3f Up;
};

CameraDirection gCameraDirections[NUM_CUBE_MAP_FACES] =
{
    { GL_TEXTURE_CUBE_MAP_POSITIVE_X, Vector3f(1.0f, 0.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f) },
    { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f) },
    { GL_TEXTURE_CUBE_MAP_POSITIVE_Y, Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 0.0f, 1.0f) },
    { GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, Vector3f(0.0f, -1.0f, 0.0f), Vector3f(0.0f, 0.0f, -1.0f) },
    { GL_TEXTURE_CUBE_MAP_POSITIVE_Z, Vector3f(0.0f, 0.0f, 1.0f),  Vector3f(0.0f, 1.0f, 0.0f) },
    { GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 1.0f, 0.0f) }
};


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

    m_pRenderingSystemGL->GetWindowSize(m_windowWidth, m_windowHeight);

    InitTechniques();

    InitShadowMapping();

    m_pickingTexture.Init(m_windowWidth, m_windowHeight);

    m_infiniteGrid.Init(SHADOW_TEXTURE_UNIT_INDEX);

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
    m_lightingTech.SetShadowCubeMapTextureUnit(SHADOW_CUBE_MAP_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetNormalMapTextureUnit(NORMAL_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetHeightMapTextureUnit(HEIGHT_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);

    if (!m_skinningTech.Init()) {
        printf("Error initializing the skinning technique\n");
        exit(1);
    }

    m_skinningTech.Enable();
    m_skinningTech.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetShadowMapTextureUnit(SHADOW_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetShadowCubeMapTextureUnit(SHADOW_CUBE_MAP_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetNormalMapTextureUnit(NORMAL_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetHeightMapTextureUnit(HEIGHT_TEXTURE_UNIT_INDEX);

    if (!m_shadowMapTech.Init()) {
        printf("Error initializing the shadow mapping technique\n");
        exit(1);
    }

    if (!m_shadowMapPointLightTech.Init()) {
        printf("Error initializing the shadow mapping point light technique\n");
        exit(1);
    }

    if (!m_flatColorTech.Init()) {
        printf("Error initializing the flat color technique\n");
        exit(1);
    }

    if (!m_pickingTech.Init()) {
        printf("Error initializing the picking technique\n");
        exit(1);
    }
}


void ForwardRenderer::InitShadowMapping()
{
    float FOV = 90.0f; // TODO: get from light
    float zNear = 1.0f;
    float zFar = 100.0f;
    PersProjInfo shadowPersProjInfo = { FOV, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, zNear, zFar };
    m_lightPersProjMatrix.InitPersProjTransform(shadowPersProjInfo);

    // Initialize an orthographic projection matrix for the directional light
    OrthoProjInfo shadowOrthoProjInfo;
    shadowOrthoProjInfo.l = -20.0f;
    shadowOrthoProjInfo.r = 20.0f;
    shadowOrthoProjInfo.t = 20.0f;
    shadowOrthoProjInfo.b = -20.0f;
    shadowOrthoProjInfo.n = -20.0f;
    shadowOrthoProjInfo.f = 20.0f;

    m_lightOrthoProjMatrix.InitOrthoProjTransform(shadowOrthoProjInfo);

    if (!m_shadowMapFBO.Init(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT)) {
        exit(1);
    }

    if (!m_shadowCubeMapFBO.Init(SHADOW_MAP_WIDTH)) {
        exit(1);
    }
}


void ForwardRenderer::SwitchToLightingTech(LIGHTING_TECHNIQUE Tech)
{
    if (m_curLightingTech != Tech) {
        switch (Tech) {
        case FORWARD_LIGHTING:
            m_pCurLightingTech = &m_lightingTech;
            break;

        case FORWARD_SKINNING:
            m_pCurLightingTech = &m_skinningTech;
            break;

        default:
            assert(0);
        }

        m_curLightingTech = Tech;
    }    

    m_pCurLightingTech->Enable();
}


void ForwardRenderer::Render(void* pWindow, GLScene* pScene, GameCallbacks* pGameCallbacks, long long TotalRuntimeMillis, long long DeltaTimeMillis)
{
    if (pScene->IsClearFrame()) {
        const Vector4f& ClearColor = pScene->GetClearColor();
        glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    pGameCallbacks->OnFrame(DeltaTimeMillis);

    if (!m_pCurCamera) {
        printf("ForwardRenderer: camera not initialized\n");
        return;
    }

    if (pScene->GetRenderList().size() == 0) {
        printf("Warning! render list is empty and no main model\n");
        return;
    }

    if (pScene->GetConfig()->IsPickingEnabled()) {
        PickingPass(pWindow, pScene);
        // The render loop may be called multiple time before picking
        // is again disabled so we do it explicitly
        pScene->GetConfig()->ControlPicking(false);
    }

    ShadowMapPass(pScene);

    LightingPass(pScene, TotalRuntimeMillis);

    m_curRenderPass = RENDER_PASS_UNINITIALIZED;
}


void ForwardRenderer::ApplySceneConfig(GLScene* pScene)
{
    m_pCurLightingTech->ControlShadows(pScene->GetConfig()->IsShadowMappingEnabled());
}


void ForwardRenderer::ApplyLighting(GLScene* pScene)
{
    int NumLightsTotal = 0;

    int NumPointLights = (int)pScene->GetPointLights().size();

    if (NumPointLights > 0) {
        m_pCurLightingTech->SetPointLights(NumPointLights, &pScene->GetPointLights()[0], true);
        NumLightsTotal += NumPointLights;
    }

    int NumSpotLights = (int)pScene->GetSpotLights().size();

    if (NumSpotLights > 0) {
        m_pCurLightingTech->SetSpotLights(NumSpotLights, &pScene->GetSpotLights()[0], true);
        NumLightsTotal += NumSpotLights;
    }

    int NumDirLights = (int)pScene->GetDirLights().size();

    if (NumDirLights > 0) {
        const DirectionalLight& DirLight = pScene->GetDirLights()[0];
        m_pCurLightingTech->SetDirectionalLight(DirLight, true);
        NumLightsTotal += NumDirLights;
    }

    if (NumLightsTotal == 0) {
        //printf("Warning! trying to render but all lights are zero\n");
        m_pCurLightingTech->ControlLighting(false);
    }
    else {
        m_pCurLightingTech->ControlLighting(true);
    }

    m_pCurLightingTech->SetCameraWorldPos(m_pCurCamera->GetPos());
}

void ForwardRenderer::PickingPass(void* pWindow, GLScene* pScene)
{
    m_curRenderPass = RENDER_PASS_PICKING;

    m_pickingTexture.EnableWriting();

    // TODO: change clear color

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_pickingTech.Enable();

    PickingRenderScene(pScene);

    m_pickingTexture.DisableWriting();

    PostPickingPass(pWindow, pScene);
}


void ForwardRenderer::PickingRenderScene(GLScene* pScene)
{
    const std::list<CoreSceneObject*>& RenderList = pScene->GetRenderList();

    for (std::list<CoreSceneObject*>::const_iterator it = RenderList.begin(); it != RenderList.end(); it++) {
        int ObjectIndex = (*it)->GetId() + 1;  // Background is zero, the real objects start at 1
        m_pickingTech.SetObjectIndex(ObjectIndex);

        m_pcurSceneObject = *it;
        RenderSingleObject(m_pcurSceneObject);
    }
}


void ForwardRenderer::PostPickingPass(void* pWindow, GLScene* pScene)
{
    int ObjectIndex = GetPickedObjectIndex(pWindow, pScene);

    if (ObjectIndex == 0) {
        pScene->SetPickedSceneObject(NULL);
    } else {
        SavePickedObject(pScene, ObjectIndex);
    }
}


int ForwardRenderer::GetPickedObjectIndex(void* pWindow, GLScene* pScene)
{
    int MousePosX = 0, MousePosY = 0;
    m_pRenderingSystemGL->GetMousePos(pWindow, MousePosX, MousePosY);
    PickingTexture::PixelInfo Pixel = m_pickingTexture.ReadPixel(MousePosX, m_windowHeight - MousePosY - 1);
    return Pixel.ObjectID;
}


void ForwardRenderer::SavePickedObject(GLScene* pScene, int ObjectIndex)
{
    const std::list<CoreSceneObject*>& RenderList = pScene->GetRenderList();

    for (std::list<CoreSceneObject*>::const_iterator it = RenderList.begin(); it != RenderList.end(); it++) {
        if (((*it)->GetId() + 1) == ObjectIndex) {
            pScene->SetPickedSceneObject(*it);
            return;
        }
    }

    // should never get here
    assert(0);
}


void ForwardRenderer::ShadowMapPass(GLScene* pScene)
{        
    const std::vector<SpotLight>& SpotLights = pScene->GetSpotLights();
    int NumSpotLights = (int)SpotLights.size();

    if (NumSpotLights > 0) {
        m_lightViewMatrix.InitCameraTransform(SpotLights[0].WorldPosition, SpotLights[0].WorldDirection, SpotLights[0].Up);
    }

    const std::vector<DirectionalLight>& DirLights = pScene->GetDirLights();
    int NumDirLights = 0;
    if (DirLights.size() == 1) {
        NumDirLights = 1;
        OrthoProjInfo LightOrthoProjInfo;
        Vector3f LightWorldPos;

        CalcTightLightProjection(m_pCurCamera->GetViewMatrix(),   // in
                                 DirLights[0].WorldDirection,     // in
                                 m_pCurCamera->GetPersProjInfo(), // in
                                 LightWorldPos,                   // out
                                 LightOrthoProjInfo);             // out

        Vector3f Up(0.0f, 1.0f, 0.0f);
        m_lightViewMatrix.InitCameraTransform(LightWorldPos, DirLights[0].WorldDirection, Up);

    } else if (DirLights.size() > 1) {
        printf("%s:%d - only a single directional light is supported\n", __FILE__, __LINE__);
    }

    int NumPointLights = (int)pScene->GetPointLights().size();

    if (NumDirLights > 0) {
        m_curRenderPass = RENDER_PASS_SHADOW_DIR;
        ShadowMapPassDirAndSpot(pScene->GetRenderList());
    } else if (NumPointLights > 0) {
        m_curRenderPass = RENDER_PASS_SHADOW_POINT;
        ShadowMapPassPoint(pScene->GetRenderList(), pScene->GetPointLights());
    } else {  
        m_curRenderPass = RENDER_PASS_SHADOW_SPOT;
        ShadowMapPassDirAndSpot(pScene->GetRenderList());
    }
}


void ForwardRenderer::ShadowMapPassPoint(const std::list<CoreSceneObject*>& RenderList, const std::vector<PointLight>& PointLights)
{
    m_shadowMapPointLightTech.Enable();
    m_shadowMapPointLightTech.SetLightWorldPos(PointLights[0].WorldPosition);

    glClearColor(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);

    for (uint i = 0; i < NUM_CUBE_MAP_FACES; i++) {
        m_shadowCubeMapFBO.BindForWriting(gCameraDirections[i].CubemapFace);
        glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        m_lightViewMatrix.InitCameraTransform(PointLights[0].WorldPosition, gCameraDirections[i].Target, gCameraDirections[i].Up);
        RenderEntireRenderList(RenderList);
    }
}


void ForwardRenderer::ShadowMapPassDirAndSpot(const std::list<CoreSceneObject*>& RenderList)
{
    m_shadowMapFBO.BindForWriting();
    glClear(GL_DEPTH_BUFFER_BIT);
    m_shadowMapTech.Enable();
    if (UseIndirectRender) {
        Matrix4f VP = m_lightPersProjMatrix * m_lightViewMatrix;
        m_shadowMapTech.SetVP(VP);
    }
    m_shadowMapTech.ControlIndirectRender(UseIndirectRender);   // TODO: same for point
    m_shadowMapTech.ControlPVP(UsePVP);                         // TODO: same for point
    RenderEntireRenderList(RenderList);
}


void ForwardRenderer::RenderEntireRenderList(const std::list<CoreSceneObject*>& RenderList)
{
    for (std::list<CoreSceneObject*>::const_iterator it = RenderList.begin(); it != RenderList.end(); it++) {
        m_pcurSceneObject = *it;
        RenderSingleObject(m_pcurSceneObject);
    }
}


void ForwardRenderer::LightingPass(GLScene* pScene, long long TotalRuntimeMillis)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    switch (m_curRenderPass) {
    case RENDER_PASS_SHADOW_DIR:
        m_shadowMapFBO.BindForReading(SHADOW_TEXTURE_UNIT);
        m_curRenderPass = RENDER_PASS_LIGHTING_DIR;
        break;

    case RENDER_PASS_SHADOW_SPOT:
        m_shadowMapFBO.BindForReading(SHADOW_TEXTURE_UNIT);
        m_curRenderPass = RENDER_PASS_LIGHTING_SPOT;
        break;

    case RENDER_PASS_SHADOW_POINT:
        m_shadowCubeMapFBO.BindForReading(SHADOW_CUBE_MAP_TEXTURE_UNIT);
        m_curRenderPass = RENDER_PASS_LIGHTING_POINT;
        break;

    default:
        // todo: handle the case when there is no shadow pass
        assert(0);
    }
   
    glViewport(0, 0, m_windowWidth, m_windowHeight);

    if (pScene->GetConfig()->GetInfiniteGrid().Enabled) {
        RenderInfiniteGrid(pScene);
    }


    bool FirstTimeForwardLighting = true;

    const std::list<CoreSceneObject*>& RenderList = pScene->GetRenderList();

    for (std::list<CoreSceneObject*>::const_iterator it = RenderList.begin(); it != RenderList.end(); it++) {        
        m_pcurSceneObject = *it;

        const Vector4f& FlatColor = m_pcurSceneObject->GetFlatColor();

        if (FlatColor.x == -1.0f) {
            if (FirstTimeForwardLighting) {
                StartRenderWithForwardLighting(pScene, m_pcurSceneObject, TotalRuntimeMillis);
              //  FirstTimeForwardLighting = false; TODO: currently disabled
            }
            RenderWithForwardLighting(m_pcurSceneObject, TotalRuntimeMillis);
        }
        else {
            RenderWithFlatColor(m_pcurSceneObject);
        }
    }
}


void ForwardRenderer::StartRenderWithForwardLighting(GLScene* pScene, CoreSceneObject* pSceneObject, long long TotalRuntimeMillis)
{
    if (pSceneObject->GetModel()->IsAnimated()) {
        SwitchToLightingTech(FORWARD_SKINNING);
    } else {
        SwitchToLightingTech(FORWARD_LIGHTING);
    }

    ApplySceneConfig(pScene);

    ApplyLighting(pScene);

    if (UseIndirectRender) {  
        Matrix4f VP = m_pCurCamera->GetVPMatrix();
        m_pCurLightingTech->SetVP(VP);
        Matrix4f LightVP = m_lightPersProjMatrix * m_lightViewMatrix;	// TODO: get the correct projection matrix
        m_pCurLightingTech->SetLightVP(LightVP);
    }

    m_pCurLightingTech->ControlIndirectRender(UseIndirectRender);
    m_pCurLightingTech->ControlPVP(UsePVP);
}


void ForwardRenderer::RenderWithForwardLighting(CoreSceneObject* pSceneObject, long long TotalRuntimeMillis)
{
    if (pSceneObject->GetModel()->IsAnimated()) {
        SwitchToLightingTech(FORWARD_SKINNING);  // TODO: do we need this?

        float AnimationTimeSec = (float)TotalRuntimeMillis / 1000.0f;
        int AnimationIndex = 0;
        vector<Matrix4f> Transforms;
        pSceneObject->GetModel()->GetBoneTransforms(AnimationTimeSec, Transforms, AnimationIndex);

        for (uint i = 0; i < Transforms.size(); i++) {
            m_skinningTech.SetBoneTransform(i, Transforms[i]);
        }
    }
    else {
        SwitchToLightingTech(FORWARD_LIGHTING);  // TODO: do we need this?
    }

    CoreModel* pModel = pSceneObject->GetModel();
    bool NormalMapEnabled = pModel->GetNormalMap() != NULL;
    bool HeightMapEnabled = pModel->GetHeightMap() != NULL;

    m_pCurLightingTech->ControlNormalMap(NormalMapEnabled);
    m_pCurLightingTech->ControlParallaxMap(HeightMapEnabled);
    m_pCurLightingTech->SetColorMod(Vector4f(pSceneObject->GetColorMod(), 1.0f)); 

    RenderSingleObject(pSceneObject);
}


void ForwardRenderer::RenderSingleObject(CoreSceneObject* pSceneObject)
{
    CoreModel* pModel = pSceneObject->GetModel();

    if (UseIndirectRender) {
        Matrix4f ObjectMatrix = m_pcurSceneObject->GetMatrix();
        pModel->RenderIndirect(ObjectMatrix);
    }
    else {
        pModel->Render(this);
    }
}


void ForwardRenderer::RenderWithFlatColor(CoreSceneObject* pSceneObject)
{
    m_curLightingTech = UNDEFINED_TECHNIQUE;    // a bit of a hack
    m_flatColorTech.Enable();
    m_flatColorTech.SetColor(pSceneObject->GetFlatColor());
    Matrix4f WVP;
    GetWVP(pSceneObject, WVP);
    m_flatColorTech.SetWVP(WVP);
    pSceneObject->GetModel()->Render();
}


void ForwardRenderer::RenderInfiniteGrid(GLScene* pScene)
{
    Matrix4f VP = m_pCurCamera->GetVPMatrix();

    Matrix4f LightVP = m_lightPersProjMatrix * m_lightViewMatrix;	// TODO: get the correct projection matrix

    const InfiniteGridConfig& Config = pScene->GetConfig()->GetInfiniteGrid();
    m_infiniteGrid.Render(Config, VP, m_pCurCamera->GetPos(), LightVP);

    // Debugging - TODO need to cleanup this mess
  /*  m_shadowMapFBO.BindForWriting();
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_windowWidth, m_windowHeight);*/
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



void ForwardRenderer::GetWVP(CoreSceneObject* pSceneObject, Matrix4f& WVP)
{
    Matrix4f World = pSceneObject->GetMatrix();
    Matrix4f View = m_pCurCamera->GetViewMatrix();
    Matrix4f Projection = m_pCurCamera->GetProjMatrix();

   // Projection.Print();
 //   exit(0);

    WVP = Projection * View * World;
}


void ForwardRenderer::ControlRimLight(bool IsEnabled)
{
    assert(0);
  //  SwitchToLightingTech();
 //   m_pCurLightingTech->ControlRimLight(IsEnabled);
}


void ForwardRenderer::ControlCellShading(bool IsEnabled)
{
    assert(0);
 //   SwitchToLightingTech();
 //   m_pCurLightingTech->ControlCellShading(IsEnabled);
}


void ForwardRenderer::SetLinearFog(float FogStart, float FogEnd, const Vector3f& FogColor)
{
    assert(0);
//    SwitchToLightingTech();
//    m_pCurLightingTech->SetLinearFog(FogStart, FogEnd);
 //   m_pCurLightingTech->SetFogColor(FogColor);
}


void ForwardRenderer::SetExpFog(float FogEnd, const Vector3f& FogColor, float FogDensity)
{
    assert(0);
 //   SwitchToLightingTech();
//    m_pCurLightingTech->SetExpFog(FogEnd, FogDensity);
//    m_pCurLightingTech->SetFogColor(FogColor);
}


void ForwardRenderer::SetExpSquaredFog(float FogEnd, const Vector3f& FogColor, float FogDensity)
{
    assert(0);
 //   SwitchToLightingTech();
 //   m_pCurLightingTech->SetExpSquaredFog(FogEnd, FogDensity);
 //   m_pCurLightingTech->SetFogColor(FogColor);
}


void ForwardRenderer::SetLayeredFog(float FogTop, float FogEnd, const Vector3f& FogColor)
{
    assert(0);
 //   SwitchToLightingTech();
 //   m_pCurLightingTech->SetLayeredFog(FogTop, FogEnd);
 //   m_pCurLightingTech->SetFogColor(FogColor);
}


void ForwardRenderer::SetAnimatedFog(float FogEnd, float FogDensity, const Vector3f& FogColor)
{
    assert(0);
//    SwitchToLightingTech();
//    m_pCurLightingTech->SetAnimatedFog(FogEnd, FogDensity);
//    m_pCurLightingTech->SetFogColor(FogColor);
}


void ForwardRenderer::UpdateAnimatedFogTime(float FogTime)
{
    assert(0);
  //  SwitchToLightingTech();
 //   m_pCurLightingTech->SetFogTime(FogTime);
}



void ForwardRenderer::DrawStart_CB(uint DrawIndex)
{
    // TODO: picking technique update
}


void ForwardRenderer::ControlSpecularExponent_CB(bool IsEnabled)
{
    switch (m_curRenderPass) {
    case RENDER_PASS_LIGHTING_DIR:
    case RENDER_PASS_LIGHTING_SPOT:
    case RENDER_PASS_LIGHTING_POINT:
        m_pCurLightingTech->ControlSpecularExponent(IsEnabled);
        break;
    }
}


void ForwardRenderer::SetMaterial_CB(const Material& material)
{
    switch (m_curRenderPass) {
    case RENDER_PASS_LIGHTING_DIR:
    case RENDER_PASS_LIGHTING_SPOT:
    case RENDER_PASS_LIGHTING_POINT:
        m_pCurLightingTech->SetMaterial(material);
    }
}


void ForwardRenderer::SetWorldMatrix_CB(const Matrix4f& World)
{
    if (UseIndirectRender) {
        printf("Incorrect callback with indirect rendering\n");
        assert(1);
    }

    switch (m_curRenderPass) {

    case RENDER_PASS_SHADOW_DIR:
        SetWorldMatrix_CB_ShadowPassDir(World);
        break;

    case RENDER_PASS_SHADOW_SPOT:
        SetWorldMatrix_CB_ShadowPassSpot(World);
        break;

    case RENDER_PASS_SHADOW_POINT:
        SetWorldMatrix_CB_ShadowPassPoint(World);
        break;

    case RENDER_PASS_LIGHTING_DIR:
    case RENDER_PASS_LIGHTING_SPOT:
    case RENDER_PASS_LIGHTING_POINT:
        SetWorldMatrix_CB_LightingPass(World);
        break;

    case RENDER_PASS_PICKING:
        SetWorldMatrix_CB_PickingPass(World);
        break;

    default:
        printf("%s:%d - Unknown render pass %d\n", __FILE__, __LINE__, m_curRenderPass);
        exit(1);
    }
}


void ForwardRenderer::SetWorldMatrix_CB_ShadowPassDir(const Matrix4f& World)
{
    Matrix4f ObjectMatrix = m_pcurSceneObject->GetMatrix();
    Matrix4f WVP = m_lightOrthoProjMatrix * m_lightViewMatrix * World * ObjectMatrix;
    m_shadowMapTech.SetWVP(WVP);
}


void ForwardRenderer::SetWorldMatrix_CB_ShadowPassSpot(const Matrix4f& World)
{
    Matrix4f ObjectMatrix = m_pcurSceneObject->GetMatrix();
    /*ObjectMatrix.Print();
    World.Print();
    m_lightViewMatrix.Print();
    m_lightPersProjMatrix.Print();
    exit(0);*/
    Matrix4f WVP = m_lightPersProjMatrix * m_lightViewMatrix * World * ObjectMatrix;
    m_shadowMapTech.SetWVP(WVP);
}


void ForwardRenderer::SetWorldMatrix_CB_ShadowPassPoint(const Matrix4f& World)
{
    Matrix4f ObjectMatrix = m_pcurSceneObject->GetMatrix();
    Matrix4f WVP = m_lightPersProjMatrix * m_lightViewMatrix * World * ObjectMatrix;
    m_shadowMapPointLightTech.SetWorld(World);
    m_shadowMapPointLightTech.SetWVP(WVP);
}


void ForwardRenderer::SetWorldMatrix_CB_LightingPass(const Matrix4f& World)
{
    Matrix4f ObjectMatrix = m_pcurSceneObject->GetMatrix();
    Matrix4f FinalWorldMatrix = World * ObjectMatrix;
    m_pCurLightingTech->SetWorldMatrix(FinalWorldMatrix);

    Matrix4f View = m_pCurCamera->GetViewMatrix();// TODO: use VP matrix from camera
    Matrix4f Projection = m_pCurCamera->GetProjMatrix();
    Matrix4f WV = View * FinalWorldMatrix;
    Matrix4f WVP = Projection * View * FinalWorldMatrix;

  //  printf("Lighting pass\n"); WVP.Print(); exit(1);

    m_pCurLightingTech->SetWVP(WVP);

    Matrix4f LightWVP;
    
    switch (m_curRenderPass) {
    case RENDER_PASS_LIGHTING_DIR:
        LightWVP = m_lightOrthoProjMatrix * m_lightViewMatrix * FinalWorldMatrix;
        break;

    case RENDER_PASS_LIGHTING_SPOT:
    case RENDER_PASS_LIGHTING_POINT:
        LightWVP = m_lightPersProjMatrix * m_lightViewMatrix * FinalWorldMatrix;
        break;

    default:
        assert(0);
    }

    m_pCurLightingTech->SetLightWVP(LightWVP);

    Matrix4f InverseWorld = FinalWorldMatrix.Inverse();
    Matrix3f World3x3(InverseWorld);
    Matrix3f WorldTranspose = World3x3.Transpose();
    //WorldTranspose.Print();
   // exit(1);
    m_pCurLightingTech->SetNormalMatrix(WorldTranspose);
}


void ForwardRenderer::SetWorldMatrix_CB_PickingPass(const Matrix4f& World)
{
    Matrix4f ObjectMatrix = m_pcurSceneObject->GetMatrix();
    Matrix4f ProjView = GetViewProjectionMatrix();
    Matrix4f WVP = ProjView * World * ObjectMatrix;
   // printf("Picking pass\n"); WVP.Print();

    m_pickingTech.SetWVP(WVP);
}


Matrix4f ForwardRenderer::GetViewProjectionMatrix()     // TODO: replace with GetVP from camera
{
    Matrix4f View = m_pCurCamera->GetViewMatrix();
    Matrix4f Projection = m_pCurCamera->GetProjMatrix();

    Matrix4f Ret = Projection * View;

    return Ret;
}

