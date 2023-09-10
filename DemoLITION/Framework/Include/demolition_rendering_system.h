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

class GameCallbacks
{
 public:

    virtual void OnFrame() {}

    virtual bool OnKeyboard(int key, int action) { return false; }

    virtual bool OnMouseMove(int x, int y) { return false; }

    virtual void OnMouseButton(int Button, int Action, int Mode) {}
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

    virtual void Shutdown() = 0;

    virtual void Execute() = 0;

    virtual Scene* CreateEmptyScene() = 0;

    virtual Scene* CreateScene(const std::string& Filename) = 0;

    virtual Scene* CreateDefaultScene() = 0;

    virtual void CreateWindow(int Width, int Height) = 0;

    virtual int LoadModel(const std::string& Filename) = 0;

    virtual void SetScene(Scene* pScene) = 0;

    virtual void GetWindowSize(int& Width, int& Height) const = 0;

    virtual long long GetElapsedTimeMillis() const = 0;
};
