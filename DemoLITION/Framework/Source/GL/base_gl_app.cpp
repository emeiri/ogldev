/*

        Copyright 2024 Etay Meiri

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


#include "demolition_base_gl_app.h"

#define GLFW_DLL
#include <GLFW/glfw3.h>



BaseGLApp::BaseGLApp(int WindowWidth, int WindowHeight)
{
    bool LoadBasicShapes = false;
    m_pRenderingSystem = RenderingSystem::CreateRenderingSystem(RENDERING_SYSTEM_GL, this, LoadBasicShapes);
    m_pRenderingSystem->CreateWindow(WindowWidth, WindowHeight);
}


bool BaseGLApp::OnKeyboard(int key, int action) 
{ 
    bool HandledByMe = true;

    switch (key) {
    case GLFW_KEY_W:
        if (action == GLFW_PRESS) {
            m_isWireframe = !m_isWireframe;
            if (m_isWireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }
        break;

    default:
        HandledByMe = false;
        break;
    }

    return HandledByMe;
}

bool OnMouseMove(int x, int y) { return false; }

void OnMouseButton(int Button, int Action, int Mode) {}
