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
*/

#ifdef VULKAN

#ifndef WIN32

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "ogldev_vulkan.h"
#include "ogldev_xcb_control.h"

XCBControl::XCBControl() 
{
    m_pXCBConn   = NULL;
    m_pXCBScreen = NULL;
}


XCBControl::~XCBControl()
{
    if (m_xcbWindow) {
        xcb_destroy_window(m_pXCBConn, m_xcbWindow);
    }
    
    if (m_pXCBConn) {
        xcb_disconnect(m_pXCBConn);
    }
}


void XCBControl::Init(uint Width, uint Height)
{
    m_pXCBConn = xcb_connect(NULL, NULL);

    int error = xcb_connection_has_error(m_pXCBConn);
    
    if  (error) {
        OGLDEV_ERROR("Error opening xcb connection error %d\n", error);
        assert(0);
    }
    
    printf("XCB connection opened\n");

    const xcb_setup_t* pSetup = xcb_get_setup(m_pXCBConn);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(pSetup);
    
    m_pXCBScreen = iter.data;    
    
    printf("XCB screen %p\n", m_pXCBScreen);        

    m_xcbWindow = xcb_generate_id(m_pXCBConn);
  
    xcb_create_window(m_pXCBConn, 
                      XCB_COPY_FROM_PARENT, 
                      m_xcbWindow,
                      m_pXCBScreen->root, 
                      0, 
                      0, 
                      Width, 
                      Height, 
                      0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, 
                      m_pXCBScreen->root_visual, 
                      0, 
                      0);

    xcb_map_window(m_pXCBConn, m_xcbWindow);    
    
    xcb_flush (m_pXCBConn);
     
    printf("Window %x created\n", m_xcbWindow);
}


VkSurfaceKHR XCBControl::CreateSurface(VkInstance& inst)
{
    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.connection = m_pXCBConn;
    surfaceCreateInfo.window = m_xcbWindow;
    
    VkSurfaceKHR surface;
    
    VkResult res = vkCreateXcbSurfaceKHR(inst, &surfaceCreateInfo, NULL, &surface);
    CHECK_VULKAN_ERROR("vkCreateXcbSurfaceKHR error %d\n", res);
    
    return surface;
}

#endif

#endif