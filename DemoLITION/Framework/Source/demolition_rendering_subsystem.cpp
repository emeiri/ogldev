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

#include "demolition_rendering_subsystem.h"
#include "GL/demolition_rendering_subsystem_gl.h"

#define NUM_MODELS 1024

BaseRenderingSubsystem* g_pRenderingSubsystem = NULL;

BaseRenderingSubsystem::BaseRenderingSubsystem(GameCallbacks* pGameCallbacks)
{
    m_pGameCallbacks = pGameCallbacks;
    m_models.resize(NUM_MODELS, 0);    
}


BaseRenderingSubsystem::~BaseRenderingSubsystem()
{

}


BaseRenderingSubsystem* BaseRenderingSubsystem::CreateRenderingSubsystem(RENDERING_SUBSYSTEM RenderingSubsystem, 
                                                                         GameCallbacks* pGameCallbacks,
                                                                         bool LoadBasicShapes)
{
    if (g_pRenderingSubsystem) {
        printf("%s:%d - rendering subsystem already exists\n", __FILE__, __LINE__);
        exit(0);
    }

    if (!pGameCallbacks) {
        printf("%s:%d - must specify game callbacks object\n", __FILE__, __LINE__);
        exit(0);
    }

    switch (RenderingSubsystem) {
    case RENDERING_SUBSYSTEM_GL:
        g_pRenderingSubsystem = new RenderingSubsystemGL(pGameCallbacks);
        g_pRenderingSubsystem->m_loadBasicShapes = LoadBasicShapes;
        break;

    default:
        printf("%s:%d - rendering system %d not supported\n", __FILE__, __LINE__, RenderingSubsystem);
        exit(0);
    }

    return g_pRenderingSubsystem;
}


void BaseRenderingSubsystem::CreateWindow(int Width, int Height)
{
    CreateWindowInternal(Width, Height);
    if (m_loadBasicShapes) {
        InitializeBasicShapes();
    }
}


void BaseRenderingSubsystem::InitializeBasicShapes()
{
    int ModelHandle = LoadModel("../Content/sphere.obj");
    m_shapeToHandle["sphere"] = ModelHandle;
    
    ModelHandle = LoadModel("../Content/box.obj");
    m_shapeToHandle["cube"] = ModelHandle;

    ModelHandle = LoadModel("../Content/quad.obj");
    m_shapeToHandle["square"] = ModelHandle;
}


Scene* BaseRenderingSubsystem::CreateDefaultScene()
{
    Scene* pScene = CreateScene();
    pScene->InitializeDefault();
    return pScene;
}


int BaseRenderingSubsystem::LoadModel(const std::string& Filename)
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
 

DemolitionModel* BaseRenderingSubsystem::GetModel(int ModelHandle)
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


DemolitionModel* BaseRenderingSubsystem::GetModel(const std::string& BasicShape)
{
    if (m_shapeToHandle.find(BasicShape) == m_shapeToHandle.end()) {
        printf("%s:%d - cannot find basic shape %s\n", __FILE__, __LINE__, BasicShape.c_str());
        exit(0);
    }

    int ModelHandle = m_shapeToHandle[BasicShape];

    DemolitionModel* pModel = m_models[ModelHandle];

    return pModel;
}