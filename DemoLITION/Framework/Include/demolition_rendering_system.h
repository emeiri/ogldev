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
#include "ogldev_glm_camera.h"
#include "demolition_scene.h"
#include "demolition_model.h"


class GameCallbacks
{
 public:

    virtual void OnFrame(long long DeltaTimeMillis) {}

    virtual void OnFrameEnd() {}

    virtual bool OnKeyboard(int key, int action) 
    { 
        return false;
    }

    virtual bool OnMouseMove(int x, int y) 
    { 
        return false; 
    }

    virtual bool OnMouseButton(int Button, int Action, int Mode, int x, int y) 
    { 
        return false; 
    }
};


enum RENDERING_SYSTEM {
    RENDERING_SYSTEM_GL,
    RENDERING_SYSTEM_VK,
    RENDERING_SYSTEM_DX12,
    RENDERING_SYSTEM_DX11
};


class RenderingSystem
{
public:

    static RenderingSystem* CreateRenderingSystem(RENDERING_SYSTEM RenderingSystem, GameCallbacks* pGameCallbacks, bool LoadBasicShapes);

    virtual void* CreateWindow(int Width, int Height, const char* pWindowName) = 0;

    virtual void ReloadShaders() = 0;

    virtual void Shutdown() = 0;

    virtual void Execute() = 0;

    virtual Scene* CreateEmptyScene() = 0;

    virtual Scene* CreateScene(const std::string& Filename) = 0;

    virtual Scene* CreateDefaultScene() = 0;

    virtual void SetScene(Scene* pScene) = 0;

    virtual Scene* GetScene() = 0;

    virtual Model* LoadModel(const std::string& Filename) = 0;

    virtual Grid* CreateGrid(int Width, int Depth) = 0;

    virtual int LoadTexture2D(const std::string& Filename) = 0;

    virtual int LoadCubemapTexture(const std::string& Filename) = 0;

    virtual void GetWindowSize(int& Width, int& Height) const = 0;

    virtual long long GetElapsedTimeMillis() const = 0;

    virtual GLMCameraFirstPerson* GetCurrentCamera() = 0;
};
