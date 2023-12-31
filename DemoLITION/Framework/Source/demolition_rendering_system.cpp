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
#include <stdio.h>
#include <stdlib.h>

#include "Int/base_rendering_system.h"
#include "Int/scene.h"
#include "GL/gl_rendering_system.h"

#define NUM_MODELS 1024

BaseRenderingSystem* g_pRenderingSystem = NULL;

BaseRenderingSystem::BaseRenderingSystem(GameCallbacks* pGameCallbacks, bool LoadBasicShapes)
{
    if (pGameCallbacks) {
        m_pGameCallbacks = pGameCallbacks;
    } else {
        printf("%s:%d - using the default game callbacks\n", __FILE__, __LINE__);
        m_pGameCallbacks = &m_defaultGameCallbacks;
    }

    m_loadBasicShapes = LoadBasicShapes;
    m_models.resize(NUM_MODELS, 0);    
}


BaseRenderingSystem::~BaseRenderingSystem()
{

}


RenderingSystem* RenderingSystem::CreateRenderingSystem(RENDERING_SYSTEM RenderingSystem, GameCallbacks* pGameCallbacks, bool LoadBasicShapes)
{
    if (g_pRenderingSystem) {
        printf("%s:%d - rendering system already exists\n", __FILE__, __LINE__);
        exit(0);
    }

    switch (RenderingSystem) {
    case RENDERING_SYSTEM_GL:
        g_pRenderingSystem = new RenderingSystemGL(pGameCallbacks, LoadBasicShapes);
        break;

    default:
        printf("%s:%d - rendering system %d not supported\n", __FILE__, __LINE__, RenderingSystem);
        exit(0);
    }

    return g_pRenderingSystem;
}


void BaseRenderingSystem::CreateWindow(int Width, int Height)
{
    m_windowWidth = Width;
    m_windowHeight = Height;

    CreateWindowInternal();

    if (m_loadBasicShapes) {
        InitializeBasicShapes();
    }
}


void BaseRenderingSystem::InitializeBasicShapes()
{
    int ModelHandle = LoadModel("../Content/sphere.obj");
    m_shapeToHandle["sphere"] = ModelHandle;
    
    ModelHandle = LoadModel("../Content/box.obj");
    m_shapeToHandle["cube"] = ModelHandle;

    ModelHandle = LoadModel("../Content/quad.obj");
    m_shapeToHandle["square"] = ModelHandle;
}


Scene* BaseRenderingSystem::CreateScene(const std::string& Filename)
{
    CoreScene* pScene = (CoreScene*)CreateEmptyScene();
    pScene->LoadScene(Filename);
    return pScene;
}


Scene* BaseRenderingSystem::CreateDefaultScene()
{
    CoreScene* pScene = (CoreScene*)CreateEmptyScene();
    pScene->InitializeDefault();
    return pScene;
}


void BaseRenderingSystem::SetScene(Scene* pScene) 
{ 
    m_pScene = pScene; 
    m_pCamera = ((CoreScene*)pScene)->GetCurrentCamera();
    SetCamera(m_pCamera);
}


int BaseRenderingSystem::LoadModel(const std::string& Filename)
{
    if (m_numModels == m_models.size()) {
        printf("%s:%d: out of models space\n", __FILE__, __LINE__);
        exit(0);
    }

    DemolitionModel* pModel = LoadModelInternal(Filename);

    int ret = -1;

    if (pModel) {
        m_models[m_numModels] = pModel;
        ret = m_numModels;
        m_numModels++;
    }
    else {
        printf("%s:%d: error loading '%s'\n", __FILE__, __LINE__, Filename.c_str());
        exit(0);
    }

    return ret;
}
 

DemolitionModel* BaseRenderingSystem::GetModel(int ModelHandle)
{
    if (ModelHandle < 0) {
        printf("%s:%d: invalid model handle %d\n", __FILE__, __LINE__, ModelHandle);
        exit(0);
    }

    if (ModelHandle >= m_numModels) {
        printf("%s:%d: invalid model handle %d\n", __FILE__, __LINE__, ModelHandle);
        exit(0);
    }

    DemolitionModel* pModel = m_models[ModelHandle];

    if (!pModel) {
        printf("%s:%d: invalid model handle %d\n", __FILE__, __LINE__, ModelHandle);
        exit(0);
    }

    return pModel;
}


DemolitionModel* BaseRenderingSystem::GetModel(const std::string& BasicShape)
{
    if (m_shapeToHandle.find(BasicShape) == m_shapeToHandle.end()) {
        printf("%s:%d - cannot find basic shape %s\n", __FILE__, __LINE__, BasicShape.c_str());
        exit(0);
    }

    int ModelHandle = m_shapeToHandle[BasicShape];

    DemolitionModel* pModel = m_models[ModelHandle];

    return pModel;
}