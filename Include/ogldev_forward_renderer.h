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


#ifndef OGLDEV_FORWARD_RENDERER
#define OGLDEV_FORWARD_RENDERER

#include "ogldev_basic_glfw_camera.h"
#include "ogldev_forward_lighting.h"
#include "ogldev_forward_skinning.h"
#include "ogldev_basic_mesh.h"
#include "ogldev_skinned_mesh.h"
#include "ogldev_shadow_mapping_technique.h"
#include "ogldev_rendering_subsystem.h"


class Scene
{
public:
    Scene() {};

    std::vector<PointLight> m_pointLights;
    std::vector<SpotLight> m_spotLights;
    std::vector<DirectionalLight> m_dirLight;
    BasicMesh* m_pMesh = NULL;
};



class ForwardRenderer {
 public:

    ForwardRenderer();

    ~ForwardRenderer();

    void InitForwardRenderer(BaseRenderingSubsystem* pRenderingSubsystem);

    void StartShadowPass();

    //void SetCamera(const BasicCamera* pCamera) { m_pCamera = pCamera; }

    //
    // Lighting
    //    

    void SetPointLights(uint NumLights, const PointLight* pPointLights);

    void SetSpotLights(uint NumLights, const SpotLight* pSpotLights);

    void UpdateDirLightDir(const Vector3f& WorldDir);

    void UpdatePointLightPos(uint Index, const Vector3f& WorldPos);

    void UpdateSpotLightPosAndDir(uint Index, const Vector3f& WorldPos, const Vector3f& WorldDir);
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

    void Render(BasicMesh* pMesh);

    void Render(const Scene& Scene);

    void RenderAnimation(SkinnedMesh* pMesh, float AnimationTimeSec, int AnimationIndex = 0);

    void RenderAnimationBlended(SkinnedMesh* pMesh,
                                float AnimationTimeSec,
                                int StartAnimIndex,
                                int EndAnimIndex,
                                float BlendFactor);

    void RenderToShadowMap(BasicMesh* pMesh, const SpotLight& SpotLight);
 
private:

    void SetDirLight(const DirectionalLight& DirLight);

    void CreateDefaultCamera();

    void GetWVP(BasicMesh* pMesh, Matrix4f& WVP);

    void SwitchToLightingTech();

    void SwitchToSkinningTech();

    void RenderAnimationCommon(SkinnedMesh* pMesh);

    void UpdateMatrices(ForwardLightingTechnique* pBaseTech, BasicMesh* pMesh);

    BaseRenderingSubsystem* m_pRenderingSubsystem = NULL;
    BasicCamera* m_pCurCamera = NULL;
    BasicCamera* m_pDefaultCamera = NULL;
    ForwardLightingTechnique m_lightingTech;
    ForwardSkinningTechnique m_skinningTech;
    ShadowMappingTechnique m_shadowMapTech;

    // Lighting info
    DirectionalLight m_dirLight;
    uint m_numPointLights = 0;
    PointLight m_pointLights[ForwardLightingTechnique::MAX_POINT_LIGHTS];
    uint m_numSpotLights = 0;
    SpotLight m_spotLights[ForwardLightingTechnique::MAX_SPOT_LIGHTS];
};

#endif
