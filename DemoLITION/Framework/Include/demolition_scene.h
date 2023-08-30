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

#include "demolition_forward_lighting.h"
#include "demolition_model.h"


class BaseRenderingSubsystem;

class SceneObject {
public:
    SceneObject() {}

    void SetModel(DemolitionModel* pModel)
    {
        if (m_pModel) {
            printf("%s:%d - model already initialized\n", __FILE__, __LINE__);
            exit(0);
        }

        m_pModel = pModel;
    }

    Matrix4f GetMatrix() const
    {
        Matrix4f Scale;
        Scale.InitScaleTransform(m_scale);

        Matrix4f Rotation;
        Rotation.InitRotateTransform(m_rot);

        Matrix4f Translation;
        Translation.InitTranslationTransform(m_pos);

        Matrix4f WorldTransformation = Translation * Rotation * Scale;

        return WorldTransformation;
    }


    DemolitionModel* GetModel() const { return m_pModel; }

    void SetPosition(float x, float y, float z) { m_pos.x = x; m_pos.y = y; m_pos.z = z; }
    void SetRotation(float x, float y, float z) { m_rot.x = x; m_rot.y = y; m_rot.z = z; }
    void SetScale(float x, float y, float z) { m_scale.x = x; m_scale.y = y; m_scale.z = z; }

    void SetPosition(const Vector3f& Pos) { m_pos = Pos; }
    void SetRotation(const Vector3f& Rot) { m_rot = Rot; }
    void SetScale(const Vector3f& Scale) { m_scale = Scale; }

    void SetFlatColor(const Vector4f Col) { m_flatColor = Col; }
    const Vector4f& GetFlatColor() const { return m_flatColor; }

private:
  //  int m_modelHandle = -1;
    DemolitionModel* m_pModel = NULL;
    Vector3f m_pos = Vector3f(0.0f, 0.0f, 0.0f);
    Vector3f m_rot = Vector3f(0.0f, 0.0f, 0.0f);
    Vector3f m_scale = Vector3f(1.0f, 1.0f, 1.0f);
    Vector4f m_flatColor = Vector4f(-1.0f, -1.0f, -1.0f, -1.0f);
};

class Scene {
public:
    Scene(BaseRenderingSubsystem* pRenderingSystem);

    virtual ~Scene() {}

    std::list<SceneObject*>& GetRenderList() { return m_renderList; }

    int CreateSceneObject(int ModelHandle);
    SceneObject* GetSceneObject(int SceneObjectHandle);
    void AddToRenderList(int SceneObjectHandle);
    bool RemoveFromRenderList(int SceneObjectHandle);

    void SetClearColor(const Vector4f& Color) { m_clearColor = Color; m_clearFrame = true; }
    void DisableClear() { m_clearFrame = false;  }
    bool IsClearFrame() const { return m_clearFrame; }
    const Vector4f& GetClearColor() { return m_clearColor;  }  

    std::vector<PointLight> m_pointLights;
    std::vector<SpotLight> m_spotLights;
    std::vector<DirectionalLight> m_dirLights;

protected:
    BaseRenderingSubsystem* m_pRenderingSystem = NULL;
    std::list<SceneObject*> m_renderList;
    bool m_clearFrame = false;
    Vector4f m_clearColor;
    std::vector<SceneObject> m_sceneObjects;
    int m_numSceneObjects = 0;
};
