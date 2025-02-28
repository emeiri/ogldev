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
    void SetRotation(float x, float y, float z);
    void SetScale(float x, float y, float z) { m_scale.x = x; m_scale.y = y; m_scale.z = z; }    

    void SetPosition(const Vector3f& Pos) { m_pos = Pos; }
    const Vector3f& GetPosition() const { return m_pos; }
    void SetRotation(const Vector3f& Rot);
    void PushRotation(const Vector3f& Rot);
    void ResetRotations() { m_numRotations = 0; }
    void SetScale(const Vector3f& Scale) { m_scale = Scale; }

    Matrix4f GetMatrix() const;

    void SetFlatColor(const Vector4f Col) { m_flatColor = Col; }
    const Vector4f& GetFlatColor() const { return m_flatColor; }

    void SetColorMod(float r, float g, float b) { m_colorMod.r = r; m_colorMod.g = g; m_colorMod.b = b; }
    Vector3f GetColorMod() const { return m_colorMod; }

    void SetQuaternion(const glm::quat& q) { m_quaternion = q; }

protected:
    SceneObject() {}
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

class SceneConfig
{
public:

    SceneConfig();

    void ControlShadowMapping(bool EnableShadowMapping) { m_shadowMappingEnabled = EnableShadowMapping; }
    bool IsShadowMappingEnabled() const { return m_shadowMappingEnabled; }

    void ControlPicking(bool EnablePicking) { m_pickingEnabled = EnablePicking; }
    bool IsPickingEnabled() const { return m_pickingEnabled; }

    InfiniteGridConfig& GetInfiniteGrid() { return m_infiniteGridConfig;  }

private:

    bool m_shadowMappingEnabled = true;
    bool m_pickingEnabled = false;
    InfiniteGridConfig m_infiniteGridConfig;
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
