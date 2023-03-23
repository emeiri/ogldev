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


#ifndef OGLDEV_RENDERING_SUBSYSTEM_H
#define OGLDEV_RENDERING_SUBSYSTEM_H

#include "ogldev_types.h"

class GameCallbacks
{
 public:

    virtual void OnFrame() {}

    virtual bool OnKeyboard(int key, int action) { return false; }

    virtual void OnMouseMove(int x, int y) {}

    virtual void OnMouseButton(int Button, int Action, int Mode) {}

    //virtual void SetCamera(BasicCamera* pCamera) {}
};


enum RENDERING_SUBSYSTEM {
    RENDERING_SUBSYSTEM_GL,
    RENDERING_SUBSYSTEM_VK,
    RENDERING_SUBSYSTEM_DX12,
    RENDERING_SUBSYSTEM_DX11
};


class BasicCamera;


class BaseRenderingSubsystem
{
 public:

    static BaseRenderingSubsystem* CreateRenderingSubsystem(RENDERING_SUBSYSTEM RenderingSubsystem, GameCallbacks* pGameCallbacks);

    virtual void Shutdown() = 0;

    virtual void CreateWindow(int Width, int Height) = 0;

    virtual void Execute() = 0;

    virtual void ClearWindow() = 0;

    void SetCamera(BasicCamera* pBasicCamera) { m_pCamera = pBasicCamera; }

    void GetWindowSize(int& Width, int& Height) const { Width = m_windowWidth; Height = m_windowHeight; }

    long long GetElapsedTimeMillis() const { return m_elapsedTimeMillis; }

 protected:

    BaseRenderingSubsystem(GameCallbacks* pGameCallbacks);
    ~BaseRenderingSubsystem();

    long long m_elapsedTimeMillis = 0;
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    BasicCamera* m_pCamera = NULL;
    GameCallbacks* m_pGameCallbacks = NULL;

 private:

};

#endif
