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

#include "ogldev_types.h"
#include "demolition_scene.h"
#include "demolition_rendering_system.h"
#include "Int/demolition_model.h"

class BasicCamera;

class BaseRenderingSystem : public RenderingSystem
{
 public:

    virtual Scene* CreateScene(const std::string& Filename);

    virtual Scene* CreateDefaultScene();

    virtual void CreateWindow(int Width, int Height);

    virtual int LoadModel(const std::string& Filename);

    virtual DemolitionModel* GetModel(int ModelHandle);

    virtual DemolitionModel* GetModel(const std::string& BasicShape);

    virtual void SetScene(Scene* pScene);

    virtual void GetWindowSize(int& Width, int& Height) const { Width = m_windowWidth; Height = m_windowHeight; }

    virtual long long GetElapsedTimeMillis() const { return m_elapsedTimeMillis; }

 protected:

    BaseRenderingSystem(GameCallbacks* pGameCallbacks, bool LoadBasicShapes);

    ~BaseRenderingSystem();

    virtual void CreateWindowInternal() = 0;

    virtual DemolitionModel* LoadModelInternal(const std::string& Filename) = 0;    

    virtual void SetCamera(BasicCamera* pCamera) = 0;

    long long m_elapsedTimeMillis = 0;
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    BasicCamera* m_pCamera = NULL;
    GameCallbacks* m_pGameCallbacks = NULL;
    GameCallbacks m_defaultGameCallbacks;
    Scene* m_pScene = NULL;

 private:
    void InitializeBasicShapes();

    std::vector<DemolitionModel*> m_models;
    std::map<std::string, int> m_shapeToHandle;
    int m_numModels = 0;
    bool m_loadBasicShapes = false;
};

