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

#include "ogldev_framebuffer.h"
#include "GL/gl_engine_common.h"
#include "GL/gl_forward_renderer.h"
#include "GL/gl_rendering_system.h"
#include "GL/gl_model.h"


#define SHADOW_MAP_WIDTH 2048
#define SHADOW_MAP_HEIGHT 2048

extern bool UsePVP;
extern bool UseIndirectRender;
static bool UseBlitForFinalCopy = true;

#define SSAO_UBO_INDEX  0
#define LIGHT_UBO_INDEX 1

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

struct SSAOParamsInternal
{
    SSAOParams params;
    float zNear = 0.f;
    float zFar = 0.0f;
    float OutputWidth = 0.0f;
    float OutputHeight = 0.0f;
};


static bool IsLightingPass(RENDER_PASS RenderPass)
{
    bool ret = false;

    switch (RenderPass)
    {
    case RENDER_PASS_LIGHTING_DIR:
    case RENDER_PASS_LIGHTING_SPOT:
    case RENDER_PASS_LIGHTING_POINT:
        ret = true;
        break;
    }

    return ret;
}


ForwardRenderer::ForwardRenderer() : m_ssaoRotTexture(GL_TEXTURE_2D)
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

    m_skybox.Init(SKYBOX_TEXTURE_UNIT, SKYBOX_TEXTURE_UNIT_INDEX);

    m_lightingFBO.Init(m_windowWidth, m_windowHeight, 4, false, true, false);

    m_normalFBO.Init(m_windowWidth, m_windowHeight, 3, false, true, false);

    m_ssaoFBO.Init(m_windowWidth, m_windowHeight, 3, false, false, false);

    m_hdrFBO.Init(m_windowWidth, m_windowHeight, 3, true, true, false);

    m_ssgiFBO.Init(m_windowWidth, m_windowHeight, 4, false, true, true);

    int Size = m_windowWidth * m_windowHeight;
    m_hdrData.resize(Size * 3);

    int local_size_x = 10;
    int local_size_y = 10;
    
    m_hdrNumGroupsX = (int)AlignUpToMultiple(m_windowWidth, local_size_x) / local_size_x;
    m_hdrNumGroupsY = (int)AlignUpToMultiple(m_windowHeight, local_size_y) / local_size_y;
    
    int NumTiles = m_hdrNumGroupsX * m_hdrNumGroupsY;

    m_luminanceBuffer.InitBuffer(NumTiles * sizeof(float), NULL, GL_MAP_READ_BIT |
                                                                 GL_CLIENT_STORAGE_BIT);

    m_ssaoParams.InitBuffer(sizeof(SSAOParamsInternal), NULL, GL_DYNAMIC_STORAGE_BIT);

    m_lightSources.resize(MAX_NUM_LIGHTS);

    m_lightParams.InitBuffer(ARRAY_SIZE_IN_BYTES(m_lightSources), NULL, GL_DYNAMIC_STORAGE_BIT);

    m_ssaoRotTexture.Load("../Content/textures/rot_texture.bmp", false);

    glUseProgram(0);
}

void ForwardRenderer::ReloadShaders()
{
    InitTechniques();
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
    m_lightingTech.SetRoughnessTextureUnit(ROUGHNESS_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetMetallicTextureUnit(METALLIC_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetAOTextureUnit(AO_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetEmissiveTextureUnit(EMISSIVE_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetSkyboxTextureUnit(SKYBOX_TEXTURE_UNIT_INDEX);

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
    m_skinningTech.SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);
    m_skinningTech.SetRoughnessTextureUnit(ROUGHNESS_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetMetallicTextureUnit(METALLIC_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetAOTextureUnit(AO_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetEmissiveTextureUnit(EMISSIVE_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetSkyboxTextureUnit(SKYBOX_TEXTURE_UNIT_INDEX);
    
    if (!m_pbrLightingTech.Init()) {
        printf("Error initializing the PBR lighting technique\n");
        exit(1);
    }

    m_pbrLightingTech.Enable();
    m_pbrLightingTech.SetAlbedoTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetAmbientOcclusionTextureUnit(AO_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetEmissiveTextureUnit(EMISSIVE_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetRoughnessTextureUnit(ROUGHNESS_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetNormalTextureUnit(NORMAL_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetEnvmapTextureUnit(ENVMAP_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetBRDF_LUTTextureUnit(BRDF_LUT_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetIrradianceTextureUnit(IRRADIANCE_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetClearCoatTextureUnit(CLEARCOAT_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetClearCoatRoughnessTextureUnit(CLEARCOAT_ROUGHNESS_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetClearCoatNormalTextureUnit(CLEARCOAT_NORMAL_TEXTURE_UNIT_INDEX);

    if (!m_pbrSkinnedTech.Init()) {
        printf("Error initializing the PBR lighting technique\n");
        exit(1);
    }

    m_pbrSkinnedTech.Enable();
    m_pbrSkinnedTech.SetAlbedoTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    m_pbrSkinnedTech.SetAmbientOcclusionTextureUnit(AO_TEXTURE_UNIT_INDEX);
    m_pbrSkinnedTech.SetEmissiveTextureUnit(EMISSIVE_TEXTURE_UNIT_INDEX);
    m_pbrSkinnedTech.SetRoughnessTextureUnit(ROUGHNESS_TEXTURE_UNIT_INDEX);
    m_pbrSkinnedTech.SetNormalTextureUnit(NORMAL_TEXTURE_UNIT_INDEX);
    m_pbrSkinnedTech.SetEnvmapTextureUnit(ENVMAP_TEXTURE_UNIT_INDEX);
    m_pbrSkinnedTech.SetBRDF_LUTTextureUnit(BRDF_LUT_TEXTURE_UNIT_INDEX);
    m_pbrSkinnedTech.SetIrradianceTextureUnit(IRRADIANCE_TEXTURE_UNIT_INDEX);
    m_pbrSkinnedTech.SetClearCoatTextureUnit(CLEARCOAT_TEXTURE_UNIT_INDEX);
    m_pbrSkinnedTech.SetClearCoatRoughnessTextureUnit(CLEARCOAT_ROUGHNESS_TEXTURE_UNIT_INDEX);
    m_pbrSkinnedTech.SetClearCoatNormalTextureUnit(CLEARCOAT_NORMAL_TEXTURE_UNIT_INDEX);

    if (!m_geometryTech.Init()) {
        printf("Error initializing the PBR lighting technique\n");
        exit(1);
    }

    m_geometryTech.Enable();
    m_geometryTech.SetAlbedoTextureUnit(COLOR_TEXTURE_UNIT_INDEX);

    if (!m_shadowMapTech.Init()) {
        printf("Error initializing the shadow mapping technique\n");
        exit(1);
    }

    if (!m_shadowMapPointLightTech.Init()) {
        printf("Error initializing the shadow mapping point light technique\n");
        exit(1);
    }

    if (!m_pickingTech.Init()) {
        printf("Error initializing the picking technique\n");
        exit(1);
    }

    if (!m_fullScreenQuadTech.Init()) {
        printf("Error initializing the full screen quad technique\n");
        exit(1);
    }

    if (!m_ssaoTech.Init()) {
        printf("Error initializing the SSAO technique\n");
        exit(1);
    }

    if (!m_ssaoCombineTech.Init()) {
        printf("Error initializing the SSAO combine technique\n");
        exit(1);
    }

    if (!m_normalTech.Init()) {
        printf("Error initializing the normal technique\n");
        exit(1);
    }

    if (!m_toneMapTech.Init()) {
        printf("Error initializing the tone mapping technique\n");
        exit(1);
    }

    m_toneMapTech.Enable();
    m_toneMapTech.SetHDRSampler(0);

    if (!m_hdrTech.Init()) {
        printf("Error initializing the HDR technique\n");
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


void ForwardRenderer::SwitchToLightingTech(LIGHTING_TECHNIQUE TechId)
{
    if (m_curLightingTechId != TechId) {
        switch (TechId) {
        case FORWARD_LIGHTING:
            m_pCurLightingTech = &m_lightingTech;
            break;

        case FORWARD_SKINNING:
            m_pCurLightingTech = &m_skinningTech;
            break;

        case PBR_GLTF2_LIGHTING:
            m_pCurLightingTech = &m_pbrLightingTech;
            break;

        case PBR_GLTF2_SKINNING:
            m_pCurLightingTech = &m_pbrSkinnedTech;
            break;

        default:
            assert(0);
        }        

        m_curLightingTechId = TechId;
    }    

    m_pCurLightingTech->Enable();
}


LIGHTING_TECHNIQUE ForwardRenderer::GetLightingTech(CoreModel* pModel)
{
    LIGHTING_TECHNIQUE LightingTech = UNDEFINED_TECHNIQUE;

    if (pModel->IsAnimated()) {
        if (pModel->IsPBR()) {
            LightingTech = PBR_GLTF2_SKINNING;
        } else {
            LightingTech = FORWARD_SKINNING;
        }        
    } else {
        if (pModel->IsPBR()) {
            LightingTech = PBR_GLTF2_LIGHTING;
        } else {
            LightingTech = FORWARD_LIGHTING;
        }
    }

    return LightingTech;
}


void ForwardRenderer::Render(void* pWindow, GLScene* pScene, GameCallbacks* pGameCallbacks, long long TotalRuntimeMillis, long long DeltaTimeMillis)
{
    // TODO: can be removed? happens also at the start of the LightingPass
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
        HandleEmptyRenderList(pScene);        
        return;
    }

    if (pScene->GetConfig()->IsPickingEnabled()) {
        PickingPass(pWindow, pScene);
        // The render loop may be called multiple time before picking
        // is again disabled so we do it explicitly
        pScene->GetConfig()->ControlPicking(false);
    }

    ExecuteRenderGraph(pScene, TotalRuntimeMillis);

    pGameCallbacks->OnFrameEnd();

    m_curRenderPass = RENDER_PASS_UNINITIALIZED;
}

void ForwardRenderer::ExecuteRenderGraph(GLScene* pScene, long long TotalRuntimeMillis)
{
    bool IsHDR = pScene->GetConfig()->IsHDREnabled();

    if (pScene->GetConfig()->IsSSGIEnabled()) {
        GBufferPass(pScene);    // Must be before the shadow map pass...
    }

    ShadowMapPass(pScene);

    LightingPass(pScene, TotalRuntimeMillis);

    float AverageLuminance = 0.0f;
    float Exposure = 0.0f;

    if (IsHDR) {
        //HDRPassCPU(AverageLuminance, Exposure);
        HDRPassGPU(AverageLuminance, Exposure);
        pScene->GetConfig()->SetHDRParams(AverageLuminance, Exposure);
    }

    if (pScene->GetConfig()->IsSkyboxEnabled()) {
        m_skybox.Render(pScene->GetSkyboxTex(), m_pCurCamera->GetVPMatrixNoTranslate());
    }

    if (pScene->GetConfig()->IsSSAOEnabled()) {
        NormalPass(pScene);
        SSAOPass(pScene);
        SSAOCombinePass();
    } else if (IsHDR) {
        ToneMappingPass(AverageLuminance, Exposure, 
                        pScene->GetConfig()->GetToneMapMethod(),
                        pScene->GetConfig()->IsGammaCorrectionEnabled());

        /*if (UseBlitForFinalCopy) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            m_lightingFBO.BlitToWindow();
        } else {
            FullScreenQuadBlit(pScene);
        }*/
    } else {
        // Do nothing?
    }
}

void ForwardRenderer::HandleEmptyRenderList(GLScene* pScene)
{
    if (pScene->GetConfig()->IsSkyboxEnabled()) {
        m_skybox.Render(pScene->GetSkyboxTex(), m_pCurCamera->GetVPMatrixNoTranslate());
    }
    else {
        printf("Warning! render list is empty and no main model or skybox\n");
    }
}


void ForwardRenderer::ApplySceneConfig(GLScene* pScene)
{
    SceneConfig* pConfig = pScene->GetConfig();

    m_pCurLightingTech->ControlShadows(pConfig->IsShadowMappingEnabled());
    m_pCurLightingTech->ControlRefRefract(pConfig->IsRefRefractEnabled());
    m_pCurLightingTech->SetReflectionFactor(pConfig->GetReflectionFactor());
    m_pCurLightingTech->SetMaterialToRefRefractFactor(pConfig->GetMatRefRefractFactor());
    m_pCurLightingTech->SetRefractETA(1.0f / pConfig->GetIndexOfRefraction());
    m_pCurLightingTech->SetFresnelPower(pConfig->GetFresnelPower());

    int EnvMap = pConfig->GetEnvMap();

    if (EnvMap >= 0) {
        CubemapTexture* pCubemapTex = (CubemapTexture*)m_pRenderingSystemGL->GetTexture(EnvMap);
        pCubemapTex->Bind(ENVMAP_TEXTURE_UNIT);
    }

    if (pConfig->pBRDF_LUT) {
        pConfig->pBRDF_LUT->Bind(BRDF_LUT_TEXTURE_UNIT);
    }

    int IrradianceMap = pConfig->GetIrradianceMap();

    if (IrradianceMap >= 0) {
        CubemapTexture* pCubemapTex = (CubemapTexture*)m_pRenderingSystemGL->GetTexture(IrradianceMap);
        pCubemapTex->Bind(IRRADIANCE_TEXTURE_UNIT);
    }
}


void ForwardRenderer::UpdateLightSources(GLScene* pScene)
{
    SetupLightSourcesArray(pScene);

    // TODO: can update only the correct number of lights
    m_lightParams.Update(m_lightSources.data(), ARRAY_SIZE_IN_BYTES(m_lightSources));
}


void ForwardRenderer::SetupLightSourcesArray(GLScene* pScene)
{
    int LightIndex = 0;

    for (const SpotLight& l : pScene->GetSpotLights()) {
        assert(LightIndex < MAX_NUM_LIGHTS);
        m_lightSources[LightIndex].LightType = LIGHT_TYPE_SPOT;
        m_lightSources[LightIndex].AmbientIntensity = l.AmbientIntensity;
        m_lightSources[LightIndex].Atten_Constant = l.Attenuation.Constant;
        m_lightSources[LightIndex].Atten_Linear = l.Attenuation.Linear;
        m_lightSources[LightIndex].Atten_Exp = l.Attenuation.Exp;
        m_lightSources[LightIndex].Color = l.Color.ToGLM();
        m_lightSources[LightIndex].Cutoff = cosf(ToRadian(l.Cutoff));
        m_lightSources[LightIndex].DiffuseIntensity = l.DiffuseIntensity;
        Vector3f Dir = l.WorldDirection;
        m_lightSources[LightIndex].Direction = Dir.Normalize().ToGLM();
        m_lightSources[LightIndex].WorldPos = l.WorldPosition.ToGLM();
        LightIndex++;
    }

    for (const DirectionalLight& l : pScene->GetDirLights()) {
        assert(LightIndex < MAX_NUM_LIGHTS);
        m_lightSources[LightIndex].LightType = LIGHT_TYPE_DIR;
        m_lightSources[LightIndex].AmbientIntensity = l.AmbientIntensity;
        m_lightSources[LightIndex].Atten_Constant = 0.0f;
        m_lightSources[LightIndex].Atten_Linear = 0.0f;
        m_lightSources[LightIndex].Atten_Exp = 0.0f;
        m_lightSources[LightIndex].Color = l.Color.ToGLM();
        m_lightSources[LightIndex].Cutoff = 0.0f;
        m_lightSources[LightIndex].DiffuseIntensity = l.DiffuseIntensity;
        Vector3f Dir = l.WorldDirection;
        m_lightSources[LightIndex].Direction = Dir.Normalize().ToGLM();
        m_lightSources[LightIndex].WorldPos = glm::vec3(0.0f);
        LightIndex++;
    }

    for (const PointLight& l : pScene->GetPointLights()) {
        assert(LightIndex < MAX_NUM_LIGHTS);
        m_lightSources[LightIndex].LightType = LIGHT_TYPE_POINT;
        m_lightSources[LightIndex].AmbientIntensity = l.AmbientIntensity;
        m_lightSources[LightIndex].Atten_Constant = l.Attenuation.Constant;
        m_lightSources[LightIndex].Atten_Linear = l.Attenuation.Linear;
        m_lightSources[LightIndex].Atten_Exp = l.Attenuation.Exp;
        m_lightSources[LightIndex].Color = l.Color.ToGLM();
        m_lightSources[LightIndex].Cutoff = 0.0f;
        m_lightSources[LightIndex].DiffuseIntensity = l.DiffuseIntensity;
        m_lightSources[LightIndex].Direction = glm::vec3(0.0f);
        m_lightSources[LightIndex].WorldPos = l.WorldPosition.ToGLM();
        LightIndex++;
    }

    m_pCurLightingTech->SetNumLights(LightIndex);
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

    int NumDirLights = (int)pScene->GetDirLights().size();

    if (NumDirLights > 0) {
        const std::vector<DirectionalLight>& DirLights = pScene->GetDirLights();

        OrthoProjInfo LightOrthoProjInfo;
        Vector3f LightWorldPos;

        CalcTightLightProjection(m_pCurCamera->GetViewMatrix(),   // in
            DirLights[0].WorldDirection,     // in
            m_pCurCamera->GetPersProjInfo(), // in
            LightWorldPos,                   // out
            LightOrthoProjInfo);             // out

        Vector3f Up(0.0f, 1.0f, 0.0f);
        m_lightViewMatrix.InitCameraTransform(LightWorldPos, DirLights[0].WorldDirection, Up);

        if (DirLights.size() > 1) {
            //printf("%s:%d - only a single directional light is supported\n", __FILE__, __LINE__);
        }
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
        glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);  // TODO: should be done in the FBO
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


void ForwardRenderer::GBufferPass(GLScene* pScene)
{
    m_curRenderPass = RENDER_PASS_GBUFFER;

    m_ssgiFBO.BindForWriting();

    m_ssgiFBO.Clear();

    m_geometryTech.Enable();

    if (UseIndirectRender) {
        Matrix4f VP = m_pCurCamera->GetVPMatrix();
        m_geometryTech.SetVP(VP);
    }

    m_geometryTech.ControlIndirectRender(UseIndirectRender);
    m_geometryTech.ControlPVP(UsePVP);

    RenderEntireRenderList(pScene->GetRenderList());
}


void ForwardRenderer::RenderEntireRenderList(const std::list<CoreSceneObject*>& RenderList)
{
    for (std::list<CoreSceneObject*>::const_iterator it = RenderList.begin(); it != RenderList.end(); it++) {
        m_pcurSceneObject = *it;
        RenderSingleObject(m_pcurSceneObject);
    }
}


void ForwardRenderer::NormalPass(GLScene* pScene)
{
    m_curRenderPass = RENDER_PASS_NORMAL;

    m_normalFBO.BindForWriting();

    m_normalFBO.Clear();

    m_normalTech.Enable();

    if (UseIndirectRender) {
        Matrix4f VP = m_pCurCamera->GetVPMatrix();
        m_normalTech.SetVP(VP);
    }

    m_normalTech.ControlIndirectRender(UseIndirectRender); 
    m_normalTech.ControlPVP(UsePVP);                         

    RenderEntireRenderList(pScene->GetRenderList());
}



void ForwardRenderer::LightingPass(GLScene* pScene, long long TotalRuntimeMillis)
{
    LightingPassFBOSetup(pScene);
    
    if (pScene->IsClearFrame()) {
        const Vector4f& ClearColor = pScene->GetClearColor();
        glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    BindShadowMaps();

    m_lightParams.BindUBO(LIGHT_UBO_INDEX);
  
    if (pScene->GetConfig()->GetInfiniteGrid().Enabled) {
        RenderInfiniteGrid(pScene);
    }

    RenderObjectList(pScene, TotalRuntimeMillis);
}


void ForwardRenderer::LightingPassFBOSetup(GLScene* pScene)
{
    if (pScene->GetConfig()->IsSSAOEnabled()) {
        if (pScene->GetConfig()->IsHDREnabled()) {
            NOT_IMPLEMENTED;
        } else {
            m_lightingFBO.BindForWriting();
        }
    } else if (pScene->GetConfig()->IsHDREnabled()) {
        m_hdrFBO.BindForWriting();
    } else {
        SetRenderToDefaultFB();
    }
}


void ForwardRenderer::HDRPassGPU(float& AvgLogLum, float& Exposure)
{
    m_hdrFBO.BindForReading(GL_TEXTURE0);
    m_luminanceBuffer.BindSSBO(1);

    m_hdrTech.Enable();
    glDispatchCompute(m_hdrNumGroupsX, m_hdrNumGroupsY, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    float* p = (float*)m_luminanceBuffer.MapForReading();

    float sum = 0.0f;

    int NumTiles = m_hdrNumGroupsX * m_hdrNumGroupsY;

    for (int i = 0; i < NumTiles; ++i) {
        sum += p[i];
    }

    m_luminanceBuffer.Unmap();

    int NumPixels = NumTiles * 100;

    AvgLogLum = sum / (float)NumPixels;

    AvgLogLum = expf(AvgLogLum);

    Exposure = 0.18f / AvgLogLum;
}


void ForwardRenderer::HDRPassCPU(float& AvgLogLum, float& Exposure)
{
    m_hdrFBO.BindForReading(GL_TEXTURE0);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, m_hdrData.data());
    
    float sum = 0.0f;

    int Size = m_windowWidth * m_windowHeight;
    
    for (int i = 0; i < Size; i++) {
        glm::vec3 Pixel(m_hdrData[i * 3 + 0],
                        m_hdrData[i * 3 + 1],
                        m_hdrData[i * 3 + 2]);

        float lum = glm::dot(Pixel, glm::vec3(0.2126f, 0.7152f, 0.0722f));
        sum += logf(lum + 0.0001f);
    }
  
    AvgLogLum = sum / Size;

    // Forgot to mention this...we calculated the log-average
    // so we need to undo it to get back the linear luminance.
    AvgLogLum = expf(AvgLogLum);
   
    Exposure = 0.18f / AvgLogLum;
}


void ForwardRenderer::BindShadowMaps()
{
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
}


void ForwardRenderer::RenderObjectList(GLScene* pScene, long long TotalRuntimeMillis)
{
    bool FirstTimeForwardLighting = true;

    const std::list<CoreSceneObject*>& RenderList = pScene->GetRenderList();

    for (std::list<CoreSceneObject*>::const_iterator it = RenderList.begin(); it != RenderList.end(); it++) {
        m_pcurSceneObject = *it;

        if (FirstTimeForwardLighting) {
            StartRenderWithForwardLighting(pScene, m_pcurSceneObject, TotalRuntimeMillis);
            //  FirstTimeForwardLighting = false; TODO: currently disabled
        }
        RenderWithForwardLighting(m_pcurSceneObject, TotalRuntimeMillis);
    }
}


void ForwardRenderer::StartRenderWithForwardLighting(GLScene* pScene, CoreSceneObject* pSceneObject, long long TotalRuntimeMillis)
{
    LIGHTING_TECHNIQUE LightingTech = GetLightingTech(pSceneObject->GetModel());

    SwitchToLightingTech(LightingTech);

    ApplySceneConfig(pScene);

    UpdateLightSources(pScene);

    if (UseIndirectRender) {  
        Matrix4f VP = m_pCurCamera->GetVPMatrix();
        m_pCurLightingTech->SetVP(VP);
        Matrix4f LightVP = m_lightPersProjMatrix * m_lightViewMatrix;	// TODO: get the correct projection matrix
        m_pCurLightingTech->SetLightVP(LightVP);
    }

    m_pCurLightingTech->ControlIndirectRender(UseIndirectRender);
    m_pCurLightingTech->ControlPVP(UsePVP);
    m_pCurLightingTech->SetCameraWorldPos(m_pCurCamera->GetPos());
}


void ForwardRenderer::RenderWithForwardLighting(CoreSceneObject* pSceneObject, long long TotalRuntimeMillis)
{
    LIGHTING_TECHNIQUE LightingTech = GetLightingTech(pSceneObject->GetModel());

    SwitchToLightingTech(LightingTech);

    if (pSceneObject->GetModel()->IsAnimated()) {
        float AnimationTimeSec = (float)TotalRuntimeMillis / 1000.0f;
        int AnimationIndex = 0;
        vector<Matrix4f> Transforms;
        pSceneObject->GetModel()->GetBoneTransforms(AnimationTimeSec, Transforms, AnimationIndex);

        for (uint i = 0; i < Transforms.size(); i++) {
            if (LightingTech == PBR_GLTF2_SKINNING) {
                m_pbrSkinnedTech.SetBoneTransform(i, Transforms[i]);
            } else {
                m_skinningTech.SetBoneTransform(i, Transforms[i]);
            }            
        }
    }

    GLModel* pModel = (GLModel*)pSceneObject->GetModel();
    bool NormalMapEnabled = pModel->GetNormalMap() != NULL;
    bool HeightMapEnabled = pModel->GetHeightMap() != NULL;

    m_pCurLightingTech->ControlNormalMap(NormalMapEnabled);
    m_pCurLightingTech->ControlParallaxMap(HeightMapEnabled);

    if (m_pCurLightingTech) {
        const Vector4f& FlatColor = m_pcurSceneObject->GetFlatColor();

        if (FlatColor.x == -1.0f) {
            m_pCurLightingTech->SetColorMod(Vector4f(pSceneObject->GetColorMod(), 1.0f));            
        } else {
            m_pCurLightingTech->SetColorMod(Vector4f(0.0f));
            m_pCurLightingTech->SetColorAdd(FlatColor);
        }        
    }

    RenderSingleObject(pSceneObject);
}


void ForwardRenderer::RenderSingleObject(CoreSceneObject* pSceneObject)
{
    GLModel* pModel = (GLModel*)pSceneObject->GetModel();

    if (UseIndirectRender) {
        Matrix4f ObjectMatrix = m_pcurSceneObject->GetMatrix();
        pModel->RenderIndirect(ObjectMatrix);
    }
    else {
        pModel->Render(this);
    }
}

void ForwardRenderer::SetRenderToDefaultFB()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_windowWidth, m_windowHeight);
}


void ForwardRenderer::RenderInfiniteGrid(GLScene* pScene)
{
    Matrix4f VP = m_pCurCamera->GetVPMatrix();

    Matrix4f LightVP = m_lightPersProjMatrix * m_lightViewMatrix;	// TODO: get the correct projection matrix

    int NumSpotLights = (int)pScene->GetSpotLights().size();
    Vector3f LightDir(0.0f, -1.0f, 0.0f);

    if (NumSpotLights > 0) {
        LightDir = pScene->GetSpotLights()[0].WorldDirection;
    }

    InfiniteGridConfig& Config = pScene->GetConfig()->GetInfiniteGrid();
    Config.ShadowsEnabled = pScene->GetConfig()->IsShadowMappingEnabled();
    m_infiniteGrid.Render(Config, VP, m_pCurCamera->GetPos(), LightVP, LightDir);

    // Debugging - TODO need to cleanup this mess
  /*  m_shadowMapFBO.BindForWriting();
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);
    SetRenderToDefaultFB();*/
}


void ForwardRenderer::SSAOPass(GLScene* pScene)
{
    glDisable(GL_DEPTH_TEST);

    m_ssaoFBO.BindForWriting();

    m_ssaoFBO.ClearColorBuffer(Vector4f(0.0f));

    m_ssaoParams.BindUBO(SSAO_UBO_INDEX);

    SSAOParamsInternal Params;
    Params.params = pScene->GetConfig()->GetSSAOParams();
    Params.zNear = m_pCurCamera->GetPersProjInfo().zNear;
    Params.zFar = m_pCurCamera->GetPersProjInfo().zFar;
    Params.OutputHeight = (float)m_ssaoFBO.GetHeight();
    Params.OutputWidth = (float)m_ssaoFBO.GetWidth();
    
    m_ssaoParams.Update(&Params, sizeof(SSAOParamsInternal));

    m_lightingFBO.BindDepthForReading(GL_TEXTURE0);
    m_ssaoRotTexture.Bind(GL_TEXTURE1);
    m_normalFBO.BindForReading(GL_TEXTURE2);    

    m_ssaoTech.Enable();

    m_ssaoTech.Render();

    glEnable(GL_DEPTH_TEST);
}


void ForwardRenderer::SSAOCombinePass()
{
    SetRenderToDefaultFB();

    m_lightingFBO.BindForReading(GL_TEXTURE0);

    m_ssaoFBO.BindForReading(GL_TEXTURE1);
    
    m_ssaoCombineTech.Enable();

    m_ssaoCombineTech.Render();
}


void ForwardRenderer::ToneMappingPass(float AverageLuminance, float Exposure, TONE_MAP_METHOD ToneMapMethod, bool EnableGamma)
{
    SetRenderToDefaultFB();

    m_hdrFBO.BindForReading(GL_TEXTURE0);
    m_luminanceBuffer.BindSSBO(1);

    m_toneMapTech.Enable();

    m_toneMapTech.SetAverageLuminance(AverageLuminance);
    m_toneMapTech.SetExposure(Exposure);
    m_toneMapTech.SetToneMapMethod(ToneMapMethod);
    m_toneMapTech.ControlGammaCorrection(EnableGamma);

    m_toneMapTech.Render();
}


void ForwardRenderer::FullScreenQuadBlit(GLScene* pScene)
{
    SetRenderToDefaultFB();

    glClear(GL_DEPTH_BUFFER_BIT);

    if (pScene->GetConfig()->IsHDREnabled()) {
        m_hdrFBO.BindForReading(GL_TEXTURE0);
    } else {
        m_lightingFBO.BindForReading(GL_TEXTURE0);
    }
    
    m_fullScreenQuadTech.Enable();

    m_fullScreenQuadTech.Render();
}


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
    Matrix4f Projection = m_pCurCamera->GetProjMatrixGLM();

   // Projection.Print();
 //   exit(0);

    WVP = Projection * View * World;
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
        if (m_pCurLightingTech) {
            m_pCurLightingTech->ControlSpecularExponent(IsEnabled);
        }
        break;
    }
}


void ForwardRenderer::SetMaterial_CB(const CoreMaterial& material)
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

    case RENDER_PASS_NORMAL:
        SetWorldMatrix_CB_NormalPass(World);
        break;

    case RENDER_PASS_GBUFFER:
        SetWorldMatrix_CB_GBufferPass(World);
        break;

    default:
        printf("%s:%d - Unknown render pass %d\n", __FILE__, __LINE__, m_curRenderPass);
        exit(1);
    }
}


void ForwardRenderer::SetWorldMatrix_CB_ShadowPassDir(const Matrix4f& World)
{
    Matrix4f ObjectMatrix = m_pcurSceneObject->GetMatrix();
    Matrix4f WVP = m_lightOrthoProjMatrix * m_lightViewMatrix * ObjectMatrix * World;
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
    Matrix4f WVP = m_lightPersProjMatrix * m_lightViewMatrix * ObjectMatrix * World;
    m_shadowMapTech.SetWVP(WVP);
}


void ForwardRenderer::SetWorldMatrix_CB_ShadowPassPoint(const Matrix4f& World)
{
    Matrix4f ObjectMatrix = m_pcurSceneObject->GetMatrix();
    Matrix4f WVP = m_lightPersProjMatrix * m_lightViewMatrix * ObjectMatrix * World;
    m_shadowMapPointLightTech.SetWorld(World);
    m_shadowMapPointLightTech.SetWVP(WVP);
}


static Matrix3f CalcNormalMatrix(const Matrix4f& World)
{
    Matrix4f InverseWorld = World.Inverse();
    Matrix3f World3x3(InverseWorld);
    Matrix3f WorldTranspose = World3x3.Transpose();
    return WorldTranspose;
}


void ForwardRenderer::SetWorldMatrix_CB_LightingPass(const Matrix4f& World)
{
    // TODO: use GetWVP instead
    Matrix4f ObjectMatrix = m_pcurSceneObject->GetMatrix();
    Matrix4f FinalWorldMatrix = ObjectMatrix * World;
    m_pCurLightingTech->SetWorldMatrix(FinalWorldMatrix);

    Matrix4f View = m_pCurCamera->GetViewMatrix();// TODO: use VP matrix from camera
    Matrix4f Projection = m_pCurCamera->GetProjMatrixGLM();
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

    Matrix3f NormalMatrix = CalcNormalMatrix(FinalWorldMatrix);
    //NormalMatrix.Print();
   // exit(1);

    m_pCurLightingTech->SetNormalMatrix(NormalMatrix);
}


void ForwardRenderer::SetWorldMatrix_CB_NormalPass(const Matrix4f& World)
{
    // TODO: use GetWVP instead
    Matrix4f ObjectMatrix = m_pcurSceneObject->GetMatrix();
    Matrix4f FinalWorldMatrix = ObjectMatrix * World;

    Matrix4f View = m_pCurCamera->GetViewMatrix();// TODO: use VP matrix from camera
    Matrix4f Projection = m_pCurCamera->GetProjMatrixGLM();
    Matrix4f WVP = Projection * View * FinalWorldMatrix;

    //  printf("Normal pass\n"); WVP.Print(); exit(1);

    m_normalTech.SetWVP(WVP);

    Matrix3f NormalMatrix = CalcNormalMatrix(FinalWorldMatrix);
    Matrix4f ViewNormalMatrix4D = View * Matrix4f(NormalMatrix);
    Matrix3f ViewNormalMatrix(ViewNormalMatrix4D);
    //NormalMatrix.Print();
   // exit(1);
    m_normalTech.SetNormalMatrix(ViewNormalMatrix);
}


void ForwardRenderer::SetWorldMatrix_CB_GBufferPass(const Matrix4f& World)
{
    // TODO: use GetWVP instead
    Matrix4f ObjectMatrix = m_pcurSceneObject->GetMatrix();
    Matrix4f FinalWorldMatrix = ObjectMatrix * World;

    Matrix4f View = m_pCurCamera->GetViewMatrix();// TODO: use VP matrix from camera
    Matrix4f Projection = m_pCurCamera->GetProjMatrixGLM();
    Matrix4f WVP = Projection * View * FinalWorldMatrix;

    //  printf("Geometry pass\n"); WVP.Print(); exit(1);

    m_geometryTech.SetWVP(WVP);

    Matrix3f NormalMatrix = CalcNormalMatrix(FinalWorldMatrix);
    Matrix4f ViewNormalMatrix4D = View * Matrix4f(NormalMatrix);
    Matrix3f ViewNormalMatrix(ViewNormalMatrix4D);
    //NormalMatrix.Print();
   // exit(1);
    m_geometryTech.SetNormalMatrix(ViewNormalMatrix);
}

void ForwardRenderer::SetWorldMatrix_CB_PickingPass(const Matrix4f& World)
{
    Matrix4f ObjectMatrix = m_pcurSceneObject->GetMatrix();
    Matrix4f ProjView = GetViewProjectionMatrix();
    Matrix4f WVP = ProjView * ObjectMatrix * World;
   // printf("Picking pass\n"); WVP.Print();

    m_pickingTech.SetWVP(WVP);
}


Matrix4f ForwardRenderer::GetViewProjectionMatrix()     // TODO: replace with GetVP from camera
{
    Matrix4f View = m_pCurCamera->GetViewMatrix();
    Matrix4f Projection = m_pCurCamera->GetProjMatrixGLM();

    Matrix4f Ret = Projection * View;

    return Ret;
}

