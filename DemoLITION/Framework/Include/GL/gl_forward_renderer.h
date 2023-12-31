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
#include "ogldev_shadow_mapping_technique.h"
#include "ogldev_shadow_mapping_technique_point_light.h"
#include "ogldev_shadow_map_fbo.h"
#include "ogldev_shadow_cube_map_fbo.h"
#include "Int/demolition_model.h"
#include "gl_forward_lighting.h"
#include "gl_scene.h"
#include "flat_color_technique.h"

enum RENDER_PASS {
    RENDER_PASS_UNINITIALIZED = 0,
    RENDER_PASS_LIGHTING = 1,
    RENDER_PASS_SHADOW = 2,    
    RENDER_PASS_SHADOW_POINT = 3,
};


class RenderingSystemGL;

class ForwardRenderer : public DemolitionRenderCallbacks {
 public:

    ForwardRenderer();

    ~ForwardRenderer();

    void InitForwardRenderer(RenderingSystemGL* pRenderingSystemGL);

    void ShadowMapPass();

    void LightingPass();

    void SetCamera(BasicCamera* pCamera) { 
        m_pCurCamera = pCamera; 
    }

    //
    // Fog
    //
    void SetLinearFog(float FogStart, float FogEnd, const Vector3f& FogColor);

    void SetExpFog(float FogEnd, const Vector3f& FogColor, float FogDensity);

    void SetExpSquaredFog(float FogEnd, const Vector3f& FogColor, float FogDensity);

    void SetLayeredFog(float FogTop, float FogEnd, const Vector3f& FogColor);

    void SetAnimatedFog(float FogEnd, float FogDensity, const Vector3f& FogColor);

    void UpdateAnimatedFogTime(float FogTime);

    void DisableFog();

    void ControlRimLight(bool IsEnabled);

    void ControlCellShading(bool IsEnabled);

    void Render(GLScene* pScene);

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

    virtual void DisableDiffuseTexture_CB();

    virtual void SetWorldMatrix_CB(const Matrix4f& World);
 
private:

    void ShadowMapPass(GLScene* pScene);
    void ShadowMapPassPoint(const std::list<CoreSceneObject*>& RenderList, const std::vector<PointLight>& PointLights);
    void ShadowMapPassDirAndSpot(const std::list<CoreSceneObject*>& RenderList);
    void LightingPass(GLScene* pScene);
    void RenderAllSceneObjects(GLScene* pScene);
    void RenderWithForwardLighting(CoreSceneObject* pSceneObject);
    void RenderWithFlatColor(CoreSceneObject* pSceneObject);
    void StartRenderWithForwardLighting(GLScene* pScene, CoreSceneObject* pSceneObject);
    void GetWVP(CoreSceneObject* pSceneObject, Matrix4f& WVP);
    void SwitchToLightingTech();
    void InitShadowMapping();
    void InitTechniques();
    void SetWorldMatrix_CB_ShadowPass(const Matrix4f& World);
    void SetWorldMatrix_CB_ShadowPassPoint(const Matrix4f& World);
    void SetWorldMatrix_CB_LightingPass(const Matrix4f& World);
    void RenderEntireRenderList(const std::list<CoreSceneObject*>& RenderList);

    RENDER_PASS m_curRenderPass = RENDER_PASS_UNINITIALIZED;
    CoreSceneObject* m_pcurSceneObject = NULL;

    //void RenderAnimationCommon(SkinnedMesh* pMesh);

    RenderingSystemGL* m_pRenderingSystemGL = NULL;    
    BasicCamera* m_pCurCamera = NULL;    

    // Shadow stuff
    ShadowMapFBO m_shadowMapFBO;
    ShadowCubeMapFBO m_shadowCubeMapFBO;
    Matrix4f m_lightPersProjMatrix;
    Matrix4f m_lightOrthoProjMatrix;
    Matrix4f m_lightViewMatrix;

    ForwardLightingTechnique m_lightingTech;
    //ForwardSkinningTechnique m_skinningTech;
    ShadowMappingTechnique m_shadowMapTech;
    ShadowMappingPointLightTechnique m_shadowMapPointLightTech;
    FlatColorTechnique m_flatColorTech;
};

