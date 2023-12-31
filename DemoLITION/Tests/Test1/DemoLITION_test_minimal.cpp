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

    DemoLITION - Forward Renderer Demo
*/

#include "demolition.h"


#define WINDOW_WIDTH  1000
#define WINDOW_HEIGHT 1000


void test_minimal()
{
    bool LoadBasicShapes = false;
    RenderingSystem* pRenderingSystem = RenderingSystem::CreateRenderingSystem(RENDERING_SYSTEM_GL, NULL, LoadBasicShapes);
    pRenderingSystem->CreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
    pRenderingSystem->Execute();
}
