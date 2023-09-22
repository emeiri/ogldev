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

#include "ogldev_basic_glfw_camera.h"
#include "demolition_lights.h"

class SceneObject {
public:
    SceneObject() {}

    void SetPosition(float x, float y, float z) { m_pos.x = x; m_pos.y = y; m_pos.z = z; }
    void SetRotation(float x, float y, float z) { m_rot.x = x; m_rot.y = y; m_rot.z = z; }
    void SetScale(float x, float y, float z) { m_scale.x = x; m_scale.y = y; m_scale.z = z; }

    void SetPosition(const Vector3f& Pos) { m_pos = Pos; }
    void SetRotation(const Vector3f& Rot) { m_rot = Rot; }
    void SetScale(const Vector3f& Scale) { m_scale = Scale; }

    Matrix4f GetMatrix() const;

    void SetFlatColor(const Vector4f Col) { m_flatColor = Col; }
    const Vector4f& GetFlatColor() const { return m_flatColor; }

private:
    Vector3f m_pos = Vector3f(0.0f, 0.0f, 0.0f);
    Vector3f m_rot = Vector3f(0.0f, 0.0f, 0.0f);
    Vector3f m_scale = Vector3f(1.0f, 1.0f, 1.0f);
    Vector4f m_flatColor = Vector4f(-1.0f, -1.0f, -1.0f, -1.0f);
};


class Scene {
public:
    Scene();

    ~Scene() {}

    virtual void LoadScene(const std::string& Filename) = 0;    

    virtual SceneObject* CreateSceneObject(int ModelHandle) = 0;

    virtual SceneObject* CreateSceneObject(const std::string& BasicShape) = 0;
       
    virtual void AddToRenderList(SceneObject* pSceneObject) = 0;

    virtual bool RemoveFromRenderList(SceneObject* pSceneObject) = 0;

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
