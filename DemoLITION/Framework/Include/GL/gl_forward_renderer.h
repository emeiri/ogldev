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

#pragma once 

#include "ogldev_basic_glfw_camera.h"
#include "ogldev_shadow_mapping_technique_point_light.h"
#include "ogldev_framebuffer_object.h"
#include "ogldev_shadow_cube_map_fbo.h"
#include "ogldev_framebuffer.h"
#include "demolition_rendering_system.h"
#include "Int/core_model.h"
#include "gl_forward_lighting.h"
#include "gl_forward_skinning.h"
#include "gl_shadow_mapping_technique.h"
#include "gl_scene.h"
#include "GL/gl_picking_texture.h"
#include "GL/gl_picking_technique.h"
#include "GL/gl_pbr_forward_lighting.h"
#include "GL/gl_infinite_grid.h"
#include "GL/gl_skybox.h"
#include "GL/gl_full_screen_quad_technique.h"
#include "GL/gl_ssao_technique.h"
#include "GL/gl_ssao_combine_technique.h"
#include "GL/gl_normal_technique.h"
#include "GL/gl_buffer.h"


enum RENDER_PASS {
    RENDER_PASS_UNINITIALIZED = 0,
    RENDER_PASS_LIGHTING_DIR = 1,
    RENDER_PASS_LIGHTING_SPOT = 2,
    RENDER_PASS_LIGHTING_POINT = 3,
    RENDER_PASS_SHADOW_DIR = 4,    
    RENDER_PASS_SHADOW_SPOT = 5,
    RENDER_PASS_SHADOW_POINT = 6,
    RENDER_PASS_PICKING = 7,
    RENDER_PASS_NORMAL = 8
};


enum LIGHTING_TECHNIQUE {
    FORWARD_LIGHTING,
    FORWARD_SKINNING,
    UNDEFINED_TECHNIQUE
};


enum LIGHT_TYPE {
    LIGHT_TYPE_DIR,
    LIGHT_TYPE_POINT,
    LIGHT_TYPE_SPOT
};


struct LightSource {
    // Offset 0
    glm::vec3 Color;                // ALL                12 bytes
    LIGHT_TYPE LightType;           // ALL                4 bytes

    // Offset 16
    glm::vec3 Direction;            // Dir and spot       12 bytes
    float AmbientIntensity;         // ALL                4 bytes

    // Offset 32
    glm::vec3 WorldPos;             // spot and point     12 bytes
    float DiffuseIntensity;         // ALL                4 bytes

    // Offset 48
    float Atten_Constant;           // Spot and point     4 bytes
    float Atten_Linear;             // Spot and point     4 bytes
    float Atten_Exp;             // Spot and point     4 bytes
    float Cutoff;                   // spot               4 bytes
};


static_assert(sizeof(LightSource) == 64, "LightSource struct must be 64 bytes");

#define MAX_NUM_LIGHTS 4

class RenderingSystemGL;

class ForwardRenderer : public DemolitionRenderCallbacks {
 public:

    ForwardRenderer();

    ~ForwardRenderer();

    void InitForwardRenderer(RenderingSystemGL* pRenderingSystemGL);

    void ReloadShaders();

    void SetCamera(GLMCameraFirstPerson* pCamera) {
        m_pCurCamera = pCamera; 
    }

    void Render(void* pWindow, GLScene* pScene, GameCallbacks* pGameCallbacks, long long TotalRuntimeMillis, long long DeltaTimeMillis);

   // void RenderAnimation(SkinnedMesh* pMesh, float AnimationTimeSec, int AnimationIndex = 0);

 /*   void RenderAnimationBlended(SkinnedMesh* pMesh,
                                float AnimationTimeSec,
                                int StartAnimIndex,
                                int EndAnimIndex,
                                float BlendFactor);*/

    //
    // Implementation of DemolitionRenderCallbacks interface
    //
    virtual void DrawStart_CB(uint DrawIndex);

    virtual void ControlSpecularExponent_CB(bool IsEnabled);

    virtual void SetMaterial_CB(const Material& material);

    virtual void SetWorldMatrix_CB(const Matrix4f& World);

    GLuint GetSSAOTextureHandle() const { return m_ssaoFBO.GetTexture(); }
 
private:

    void HandleEmptyRenderList(GLScene* pScene);
    void ExecuteRenderGraph(GLScene* pScene, long long TotalRuntimeMillis);
    void PickingPass(void* pWindow, GLScene* pScene);
    void PickingRenderScene(GLScene* pScene);
    int GetPickedObjectIndex(void* pWindow, GLScene* pScene);
    void PostPickingPass(void* pWindow, GLScene* pScene);
    void SavePickedObject(GLScene* pScene, int ObjectIndex);
    void ShadowMapPass(GLScene* pScene);
    void ShadowMapPassPoint(const std::list<CoreSceneObject*>& RenderList, const std::vector<PointLight>& PointLights);
    void ShadowMapPassDirAndSpot(const std::list<CoreSceneObject*>& RenderList);
    void NormalPass(GLScene* pScene);
    void LightingPass(GLScene* pScene, long long TotalRuntimeMillis);
    void UpdateLightSources(GLScene* pScene);
    void SetupLightSourcesArray(GLScene* pScene);
    void SSAOPass(GLScene* pScene);
    void SSAOCombinePass();
    void FullScreenQuadBlit();
    void BindShadowMaps();
    void RenderObjectList(GLScene* pScene, long long TotalRuntimeMillis);
    void RenderWithForwardLighting(CoreSceneObject* pSceneObject, long long TotalRuntimeMillis);
    void StartRenderWithForwardLighting(GLScene* pScene, CoreSceneObject* pSceneObject, long long TotalRuntimeMillis);
    void RenderInfiniteGrid(GLScene* pScene);
    void GetWVP(CoreSceneObject* pSceneObject, Matrix4f& WVP);
    void SwitchToLightingTech(LIGHTING_TECHNIQUE Tech);
    void ApplySceneConfig(GLScene* pScene);
    void InitShadowMapping();
    void InitTechniques();
    void SetWorldMatrix_CB_ShadowPassDir(const Matrix4f& World);
    void SetWorldMatrix_CB_ShadowPassSpot(const Matrix4f& World);
    void SetWorldMatrix_CB_ShadowPassPoint(const Matrix4f& World);
    void SetWorldMatrix_CB_LightingPass(const Matrix4f& World);
    void SetWorldMatrix_CB_PickingPass(const Matrix4f& World);
    void SetWorldMatrix_CB_NormalPass(const Matrix4f& World);
    void RenderEntireRenderList(const std::list<CoreSceneObject*>& RenderList);
    Matrix4f GetViewProjectionMatrix();
    void RenderSingleObject(CoreSceneObject* pSceneObject);

    int m_windowWidth = -1;
    int m_windowHeight = -1;

    RENDER_PASS m_curRenderPass = RENDER_PASS_UNINITIALIZED;
    CoreSceneObject* m_pcurSceneObject = NULL;

    RenderingSystemGL* m_pRenderingSystemGL = NULL;    
    GLMCameraFirstPerson* m_pCurCamera = NULL;

    Framebuffer m_lightingFBO;
    Framebuffer m_normalFBO;
    Framebuffer m_ssaoFBO;
    GLBuffer m_ssaoParams;
    Texture m_ssaoRotTexture;
    GLBuffer m_lightParams;
    std::vector<LightSource> m_lightSources;

    // Shadow stuff
    FramebufferObject m_shadowMapFBO;
    ShadowCubeMapFBO m_shadowCubeMapFBO;
    Matrix4f m_lightPersProjMatrix;
    Matrix4f m_lightOrthoProjMatrix;
    Matrix4f m_lightViewMatrix;

    LIGHTING_TECHNIQUE m_curLightingTech = UNDEFINED_TECHNIQUE;
    BaseLightingTechnique* m_pCurBaseLightingTech = &m_lightingTech;
    ForwardLightingTechnique* m_pCurLightingTech = &m_lightingTech;
    ForwardLightingTechnique m_lightingTech;    
    ForwardSkinningTechnique m_skinningTech;
    NormalTechnique m_normalTech;
    ShadowMappingTechnique m_shadowMapTech;
    ShadowMappingPointLightTechnique m_shadowMapPointLightTech;
    PickingTechnique m_pickingTech;
    PBRForwardLightingTechnique m_pbrLightingTech;
    PickingTexture m_pickingTexture;
    FullScreenQuadTechnique m_fullScreenQuadTech;
    SSAOTechnique m_ssaoTech;
    SSAOCombineTechnique m_ssaoCombineTech;

    InfiniteGrid m_infiniteGrid;

    SkyBox m_skybox;
};

