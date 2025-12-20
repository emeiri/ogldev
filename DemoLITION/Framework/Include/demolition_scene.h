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

#include <list>

#include "ogldev_glm_camera.h"
#include "demolition_lights.h"
#include "demolition_model.h"

// Nobody needs more than 640k
#define MAX_NUM_ROTATIONS 8

class SceneObject : public Object {
public:
    void SetPosition(float x, float y, float z) { m_pos.x = x; m_pos.y = y; m_pos.z = z; }
    void SetPosition(const Vector3f& Pos) { m_pos = Pos; }
    void SetPosition(const SceneObject* pOtherObject) { m_pos = pOtherObject->GetPosition(); }
    void TranslateBy(float x, float y, float z) { m_pos.x += x; m_pos.y += y; m_pos.z += z; }

    void SetRotation(float x, float y, float z);
    void SetRotation(const Vector3f& Rot);
    void PushRotation(const Vector3f& Rot);
    void ResetRotations() { m_numRotations = 0; }
    void RotateBy(float x, float y, float z);

    void SetScale(float x, float y, float z) { m_scale.x = x; m_scale.y = y; m_scale.z = z; }    
    void SetScale(const Vector3f& Scale) { m_scale = Scale; }
    
    const Vector3f& GetPosition() const { return m_pos; }        
    const glm::vec3 GetGLMPos() const { return glm::vec3(m_pos.x, m_pos.y, m_pos.z); }
    Matrix4f GetMatrix() const;

    void SetFlatColor(const Vector4f Col) { m_flatColor = Col; }
    const Vector4f& GetFlatColor() const { return m_flatColor; }

    void SetColorMod(float r, float g, float b) { m_colorMod.r = r; m_colorMod.g = g; m_colorMod.b = b; }
    Vector3f GetColorMod() const { return m_colorMod; }

    void SetQuaternion(const glm::quat& q) { m_quaternion = q; }

protected:
    SceneObject();
    void CalcRotationStack(Matrix4f& Rot) const;

    Vector3f m_pos = Vector3f(0.0f, 0.0f, 0.0f);
    Vector3f m_scale = Vector3f(1.0f, 1.0f, 1.0f);

private:
   
    Vector3f m_rotations[MAX_NUM_ROTATIONS];
    int m_numRotations = 0;
    Vector4f m_flatColor = Vector4f(-1.0f, -1.0f, -1.0f, -1.0f);
    Vector3f m_colorMod = Vector3f(1.0f, 1.0f, 1.0f);
    glm::quat m_quaternion = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
};


struct InfiniteGridConfig {
    bool Enabled = false;
    float Size = 100.0f;
    float CellSize = 0.025f;
    Vector4f ColorThin = Vector4f(0.5f, 0.5f, 0.5f, 1.0f);
    Vector4f ColorThick = Vector4f(0.0f, 0.0f, 0.0f, 1.0f);
    float MinPixelsBetweenCells = 2.0f;
    bool ShadowsEnabled = false;
};

struct SSAOParams
{
    float scale = 1.0f;
    float bias = 0.2f;
    float radius = 0.2f;
    float occScale = 25.0f;
    float attScale = 1.0f;
    float distScale = 0.5f;
};


enum TONE_MAP_METHOD {
    TONE_MAP_METHOD_NONE = 0,
    TONE_MAP_METHOD_REINHARD = 1,
    TONE_MAP_METHOD_BRUNO_OPSENICA = 2,    // https://bruop.github.io/tonemapping/
    TONE_MAP_METHOD_WITH_EXPOSURE = 3
};


class SceneConfig
{
public:

    SceneConfig();

    void ControlShadowMapping(bool EnableShadowMapping) { m_shadowMappingEnabled = EnableShadowMapping; }
    bool IsShadowMappingEnabled() const { return m_shadowMappingEnabled; }

    void ControlPicking(bool EnablePicking) { m_pickingEnabled = EnablePicking; }
    bool IsPickingEnabled() const { return m_pickingEnabled; }

    void ControlSkybox(bool EnableSkybox) { m_skyboxEnabled = EnableSkybox; }
    bool IsSkyboxEnabled() const { return m_skyboxEnabled; }

    InfiniteGridConfig& GetInfiniteGrid() { return m_infiniteGridConfig;  }

    void ControlRefRefract(bool Enable) { m_refRefractEnabled = Enable; }
    bool IsRefRefractEnabled() const { return m_refRefractEnabled; }

    void SetReflectionFactor(float f) { m_reflectionFactor = std::min(1.0f, std::max(f, 0.0f)); }
    float GetReflectionFactor() const { return m_reflectionFactor; }

    void SetMatRefRefractFactor(float f) { m_matRefRefractFactor = std::min(1.0f, std::max(f, 0.0f)); }
    float GetMatRefRefractFactor() const { return m_matRefRefractFactor; }

    void SetIndexOfRefraction(float f) { m_indexOfRefraction = f; }
    float GetIndexOfRefraction() const { return m_indexOfRefraction; }

    void SetFresnelPower(float f) { m_fresnelPower = f; }
    float GetFresnelPower() const { return m_fresnelPower; }

    void SetEnvMap(int EnvMap) { m_envMap = EnvMap; }
    int GetEnvMap() const { return m_envMap; }

    void SetIrradianceMap(int IrradianceMap) { m_irradianceMap = IrradianceMap; }
    int GetIrradianceMap() const { return m_irradianceMap; }

    SSAOParams& GetSSAOParams() { return m_ssaoParams; }

    void ControlSSAO(bool Enable) { m_ssaoEnabled = Enable; }
    bool IsSSAOEnabled() const { return m_ssaoEnabled; }

    void ControlHDR(bool Enable) { m_hdrEnabled = Enable; }
    bool IsHDREnabled() const { return m_hdrEnabled; }

    void ControlSSGI(bool Enable) { m_ssgiEnabled = Enable; }
    bool IsSSGIEnabled() const { return m_ssgiEnabled; }

    void SetHDRParams(float AverageLuminance, float Exposure) { m_hdrAverageLuminance = AverageLuminance; m_hdrExposure = Exposure; }

    void GetHDRParams(float& AverageLuminance, float& Exposure) { AverageLuminance = m_hdrAverageLuminance; Exposure = m_hdrExposure; }

    TONE_MAP_METHOD GetToneMapMethod() const { return m_toneMapMethod; }

    void SetToneMapMethod(TONE_MAP_METHOD Method) { m_toneMapMethod = Method; }

    void ControlGammaCorrection(bool Enable) { m_enableGamma = Enable; }

    bool IsGammaCorrectionEnabled() const { return m_enableGamma; }

    Texture* pBRDF_LUT = NULL;      // TODO: should be in the material - for some reason crashes...

private:

    bool m_shadowMappingEnabled = false;
    bool m_pickingEnabled = false;
    bool m_skyboxEnabled = false;
    InfiniteGridConfig m_infiniteGridConfig;
    bool m_refRefractEnabled = false;
    float m_reflectionFactor = 0.1f;
    float m_matRefRefractFactor = 0.5f;
    float m_indexOfRefraction = 1.0f;
    float m_fresnelPower = 1.0f;
    int m_envMap = -1;
    int m_irradianceMap = -1;
    SSAOParams m_ssaoParams;
    bool m_ssaoEnabled = false;
    bool m_hdrEnabled = false;
    float m_hdrAverageLuminance = 0.0f;
    float m_hdrExposure = 0.0f;
    TONE_MAP_METHOD m_toneMapMethod = TONE_MAP_METHOD_WITH_EXPOSURE;
    bool m_enableGamma = false;
    bool m_ssgiEnabled = false;
};


struct CameraState {

};

class Scene {
public:
    Scene();

    ~Scene() {}

    virtual SceneObject* CreateSceneObject(Model* pModel) = 0;

    virtual SceneObject* CreateSceneObject(const std::string& BasicShape) = 0;
       
    virtual void AddToRenderList(SceneObject* pSceneObject) = 0;

    virtual bool RemoveFromRenderList(SceneObject* pSceneObject) = 0;

    virtual std::list<SceneObject*> GetSceneObjectsList() = 0;

    virtual void SetCamera(const Vector3f& Pos, const Vector3f& Target) = 0;

    virtual GLMCameraFirstPerson* GetCurrentCamera() = 0;
    
    virtual void SetCameraSpeed(float Speed) = 0;

    virtual SceneObject* GetPickedSceneObject() const = 0;

    virtual SceneConfig* GetConfig() = 0;

    virtual void LoadSkybox(const char* pFilename) = 0;

    virtual void ShowSceneGUI() = 0;

    std::vector<PointLight>& GetPointLights() { return m_pointLights; }

    std::vector<SpotLight>& GetSpotLights() { return m_spotLights; }

    std::vector<DirectionalLight>& GetDirLights() { return m_dirLights; }

    void SetClearColor(const Vector4f& Color) { m_clearColor = Color; m_clearFrame = true; }

    void DisableClear() { m_clearFrame = false;  }   

protected:
    bool m_clearFrame = false;
    Vector4f m_clearColor;

    std::vector<PointLight> m_pointLights;
    std::vector<SpotLight> m_spotLights;
    std::vector<DirectionalLight> m_dirLights;
};
