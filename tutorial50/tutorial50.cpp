/*

	Copyright 2016 Etay Meiri

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

    Tutorial 50 - Vulkan
*/

#include <cfloat>
#include <math.h>
#include <GL/glew.h>
#include <string>
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif
#include <sys/types.h>
#include <vector>

#include "ogldev_engine_common.h"
#include "ogldev_app.h"
#include "ogldev_util.h"
#include "ogldev_vulkan_core.h"
#ifdef WIN32
#include "ogldev_win32_control.h"
#else
#include "ogldev_xcb_control.h"
#endif

#define WINDOW_WIDTH  1024  
#define WINDOW_HEIGHT 1024

const char* pAppName = "tutorial50";


int main(int argc, char** argv)
{    
    VulkanWindowControl* pWindowControl = NULL;
#ifdef WIN32
    pWindowControl = new Win32Control(pAppName);
#else            
    pWindowControl = new XCBControl();
#endif    
    pWindowControl->Init(WINDOW_WIDTH, WINDOW_HEIGHT);

    OgldevVulkanCore core(pAppName);
    core.Init(pWindowControl);
    
    return 0;
}