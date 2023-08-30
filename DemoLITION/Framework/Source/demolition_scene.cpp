/*

        Copyright 2023 Etay Meiri

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

#include "demolition_scene.h"
#include "demolition_rendering_subsystem.h"

#define NUM_SCENE_OBJECTS 1024

Scene::Scene(BaseRenderingSubsystem* pRenderingSystem)
{ 
    m_pRenderingSystem = pRenderingSystem; 
    m_sceneObjects.resize(NUM_SCENE_OBJECTS);
}

void Scene::AddObject(int SceneObjectHandle)
{
    SceneObject* pSceneObject = GetSceneObject(SceneObjectHandle);
    std::list<SceneObject*>::const_iterator it = std::find(m_renderList.begin(), m_renderList.end(), pSceneObject);

    if (it == m_renderList.end()) {
        m_renderList.push_back(pSceneObject);
    }
}


bool Scene::RemoveObject(int SceneObjectHandle)
{
    SceneObject* pSceneObject = GetSceneObject(SceneObjectHandle);
    std::list<SceneObject*>::const_iterator it = std::find(m_renderList.begin(), m_renderList.end(), pSceneObject);

    bool ret = false;

    if (it != m_renderList.end()) {
        m_renderList.erase(it);
        ret = true;
    }

    return ret;
}


int Scene::CreateSceneObject(int ModelHandle)
{
    if (ModelHandle < 0) {
        printf("%s:%d - invalid model handle %d\n", __FILE__, __LINE__, ModelHandle);
        exit(0);
    }

    int ret = -1;

    if (m_numSceneObjects == NUM_SCENE_OBJECTS) {
        printf("%s:%d - out of scene objects space, model handle %d\n", __FILE__, __LINE__, ModelHandle);
        exit(0);
    }
    
    DemolitionModel* pModel = m_pRenderingSystem->GetModel(ModelHandle);
    m_sceneObjects[m_numSceneObjects].SetModel(pModel);

    ret = m_numSceneObjects;
    m_numSceneObjects++;
    
    return ret;
}


SceneObject* Scene::GetSceneObject(int SceneObjectHandle)
{
    if (SceneObjectHandle < 0) {
        printf("%s:%d: invalid model handle %d\n", __FILE__, __LINE__, SceneObjectHandle);
        exit(0);
    }

    if (SceneObjectHandle >= m_numSceneObjects) {
        printf("%s:%d: invalid model handle %d\n", __FILE__, __LINE__, SceneObjectHandle);
        exit(0);
    }

    SceneObject* p = &m_sceneObjects[SceneObjectHandle];

    return p;
}