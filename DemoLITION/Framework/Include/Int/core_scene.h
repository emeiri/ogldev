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

#include "demolition_scene.h"
#include "demolition_object.h"
#include "Int\core_model.h"
#include "ogldev_physics.h"


class CoreSceneObject : public SceneObject {
public:
    CoreSceneObject() {}

    void SetModel(CoreModel* pModel)
    {
        if (m_pModel) {
            printf("%s:%d - model already initialized\n", __FILE__, __LINE__);
            exit(0);
        }

        m_pModel = pModel;
    }

    CoreModel* GetModel() const { return m_pModel; }

    void SetId(int id) { m_id = id; }

    int GetId() const { return m_id; }

    void SetParticle(OgldevPhysics::Particle* pParticle) { m_pParticle = pParticle; }

    virtual Matrix4f GetMatrix();

    virtual void SetMass(float Mass);

    virtual void SetVelocity(const Vector3f& Velocity);
   
    virtual void SetAcceleration(const Vector3f& Acceleration);
    
    virtual void SetDamping(float Damping);

protected:
    virtual void UpdatePosition();

private:
    CoreModel* m_pModel = NULL;
    int m_id = -1;
    OgldevPhysics::Particle* m_pParticle = NULL;
};


class CoreRenderingSystem;

/*class CoreSceneConfig : public SceneConfig()
{
public:
    CoreSceneConfig() {}
}*/

class CoreScene : public Scene, public Object {
public:
    CoreScene(CoreRenderingSystem* pRenderingSystem);

    virtual ~CoreScene() {}

    virtual void LoadScene(const std::string& Filename);

    virtual SceneObject* CreateSceneObject(Model* pModel);

    virtual SceneObject* CreateSceneObject(const std::string& BasicShape);

    virtual std::list<SceneObject*> GetSceneObjectsList();

    const std::vector<PointLight>& GetPointLights();

    const std::vector<SpotLight>& GetSpotLights();

    const std::vector<DirectionalLight>& GetDirLights();

    BasicCamera* GetCurrentCamera() { return &m_defaultCamera; }

    void InitializeDefault();    

    const std::list<CoreSceneObject*>& GetRenderList() { return m_renderList; }
    
    void AddToRenderList(SceneObject* pSceneObject);

    bool RemoveFromRenderList(SceneObject* pSceneObject);

    bool IsClearFrame() const { return m_clearFrame; }

    const Vector4f& GetClearColor() { return m_clearColor; }

    void SetCamera(const Vector3f& Pos, const Vector3f& Target);

    void SetCameraSpeed(float Speed);

    void SetPickedSceneObject(CoreSceneObject* pSceneObject) { m_pPickedSceneObject = pSceneObject; }

    SceneObject* GetPickedSceneObject() const { return m_pPickedSceneObject; }

    SceneConfig* GetConfig() { return &m_config; }

    void Update(long long dt);

protected:
    CoreRenderingSystem* m_pCoreRenderingSystem = NULL;
    std::list<CoreSceneObject*> m_renderList;
    OgldevPhysics::PhysicsSystem m_physicsSystem;

private:
    void CreateDefaultCamera();
    CoreSceneObject* CreateSceneObjectInternal(CoreModel* pModel);

    BasicCamera m_defaultCamera;
    std::vector<CoreSceneObject> m_sceneObjects;
    int m_numSceneObjects = 0;
    CoreSceneObject* m_pPickedSceneObject = NULL;
    SceneConfig m_config;
};
