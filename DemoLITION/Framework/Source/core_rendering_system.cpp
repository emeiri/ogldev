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

#include "Int/core_rendering_system.h"
#include "Int/core_scene.h"
#include "GL/gl_rendering_system.h"

#define NUM_MODELS 1024

CoreRenderingSystem* g_pRenderingSystem = NULL;

CoreRenderingSystem::CoreRenderingSystem(GameCallbacks* pGameCallbacks, bool LoadBasicShapes)
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


CoreRenderingSystem::~CoreRenderingSystem()
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


void* CoreRenderingSystem::CreateWindow(int Width, int Height)
{
    m_windowWidth = Width;
    m_windowHeight = Height;

    void* pWindow = CreateWindowInternal();

    if (m_loadBasicShapes) {
        InitializeBasicShapes();
    }

    return pWindow;
}


void CoreRenderingSystem::InitializeBasicShapes()
{
    CoreModel* pModel = LoadModelInternal("../Content/sphere.obj");
    m_shapeToModel["sphere"] = pModel;
    
    pModel = LoadModelInternal("../Content/box.obj");
    m_shapeToModel["cube"] = pModel;

    pModel = LoadModelInternal("../Content/quad.obj");
    m_shapeToModel["square"] = pModel;
}


Scene* CoreRenderingSystem::CreateScene(const std::string& Filename)
{
    CoreScene* pScene = (CoreScene*)CreateEmptyScene();
    pScene->LoadScene(Filename);
    return pScene;
}


Scene* CoreRenderingSystem::CreateDefaultScene()
{
    CoreScene* pScene = (CoreScene*)CreateEmptyScene();
    pScene->InitializeDefault();
    return pScene;
}


void CoreRenderingSystem::SetScene(Scene* pScene) 
{ 
    m_pScene = pScene; 
    m_pCamera = ((CoreScene*)pScene)->GetCurrentCamera();
    SetCamera(m_pCamera);
}


Scene* CoreRenderingSystem::GetScene()
{
    return m_pScene;
}


Model* CoreRenderingSystem::LoadModel(const std::string& Filename)
{
    if (m_numModels == m_models.size()) {
        printf("%s:%d: out of models space\n", __FILE__, __LINE__);
        exit(0);
    }

    CoreModel* pModel = LoadModelInternal(Filename);

    if (pModel) {
        m_models[m_numModels] = pModel;
        m_numModels++;
        pModel->SetName(Filename);
    }
    else {
        printf("%s:%d: error loading '%s'\n", __FILE__, __LINE__, Filename.c_str());
        exit(0);
    }

    return pModel;
}
 

Grid* CoreRenderingSystem::CreateGrid(int Width, int Depth)
{
    if (m_numModels == m_models.size()) {
        printf("%s:%d: out of models space\n", __FILE__, __LINE__);
        exit(0);
    }

    Grid* pGrid = CreateGridInternal(Width, Depth);

  /*  if (pGrid) {
        m_models[m_numModels] = pGrid;
        m_numModels++;
    }
    else {
        printf("%s:%d: error creating grid\n", __FILE__, __LINE__);
        exit(0);
    }*/

    return pGrid;

}


Model* CoreRenderingSystem::GetModel(const std::string& BasicShape)
{
    if (m_shapeToModel.find(BasicShape) == m_shapeToModel.end()) {
        printf("%s:%d - cannot find basic shape %s\n", __FILE__, __LINE__, BasicShape.c_str());
        exit(0);
    }

    Model* pModel = m_shapeToModel[BasicShape];

    return pModel;
}