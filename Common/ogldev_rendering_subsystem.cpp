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

#include "ogldev_rendering_subsystem.h"
#include "ogldev_rendering_subsystem_gl.h"

BaseRenderingSubsystem* g_pRenderingSubsystem = NULL;

BaseRenderingSubsystem::BaseRenderingSubsystem(GameCallbacks* pGameCallbacks)
{
    m_pGameCallbacks = pGameCallbacks;
}

BaseRenderingSubsystem::~BaseRenderingSubsystem()
{

}


BaseRenderingSubsystem* BaseRenderingSubsystem::CreateRenderingSubsystem(RENDERING_SUBSYSTEM RenderingSubsystem, GameCallbacks* pGameCallbacks)
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
        break;

    default:
        printf("%s:%d - rendering system %d not supported\n", __FILE__, __LINE__, RenderingSubsystem);
        exit(0);
    }

    return g_pRenderingSubsystem;
}
