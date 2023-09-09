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
#include "demolition_rendering_subsystem.h"
#include "demolition_forward_lighting.h"
#include "demolition_model.h"
#include "GL/gl_scene.h"
#include "GL/flat_color_technique.h"


class ForwardRenderer : public DemolitionRenderCallbacks {
 public:

    ForwardRenderer();

    ~ForwardRenderer();

    void InitForwardRenderer(BaseRenderingSubsystem* pRenderingSubsystem);

    void StartShadowPass();

    void SetCamera(BasicCamera* pCamera) { m_pCurCamera = pCamera; }

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

    void RenderToShadowMap(SceneObject* pSceneObject, const SpotLight& SpotLight);

    // Implementation of DemolitionRenderCallbacks interface
    virtual void DrawStartCB(uint DrawIndex);

    virtual void ControlSpecularExponent(bool IsEnabled);

    virtual void SetMaterial(const Material& material);

    virtual void DisableDiffuseTexture();

    virtual void SetWorldMatrix(const Matrix4f& World);
 
private:

    void RenderAllSceneObjects(GLScene* pScene);
    void RenderWithForwardLighting(GLScene* pScene, SceneObject* pSceneObject);
    void RenderWithFlatColor(GLScene* pScene, SceneObject* pSceneObject);
    void StartRenderWithForwardLighting(GLScene* pScene, SceneObject* pSceneObject);

    void GetWVP(SceneObject* pSceneObject, Matrix4f& WVP);

    void SwitchToLightingTech();

    //void RenderAnimationCommon(SkinnedMesh* pMesh);

    BaseRenderingSubsystem* m_pRenderingSubsystem = NULL;
    BasicCamera* m_pCurCamera = NULL;    
    ForwardLightingTechnique m_lightingTech;
    //ForwardSkinningTechnique m_skinningTech;
    ShadowMappingTechnique m_shadowMapTech;
    FlatColorTechnique m_flatColorTech;
};

