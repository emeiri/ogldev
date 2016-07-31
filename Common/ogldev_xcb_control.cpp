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

#include <stdio.h>
#include <stdlib.h>

//#include "ogldev_vulkan.h"
#include "ogldev_xcb_control.h"

XCBControl::XCBControl() 
{
    m_pXCBConn   = NULL;
    m_pXCBScreen = NULL;
    m_pXCBDelWin = NULL;
}


XCBControl::~XCBControl()
{
    
    
}


bool XCBControl::Init(uint Width, uint Height)
{
    const xcb_setup_t *setup;
    xcb_screen_iterator_t iter;
    int scr;

    m_pXCBConn = xcb_connect(NULL, &scr);
    
    if (m_pXCBConn == NULL) {
        printf("Error opening xcb connection\n");
        return false;
    }
    
    printf("XCB connection opened\n");

    setup = xcb_get_setup(m_pXCBConn);
    iter = xcb_setup_roots_iterator(setup);
    while (scr-- > 0)
        xcb_screen_next(&iter);

    m_pXCBScreen = iter.data;    
    
    printf("XCB screen %p\n", m_pXCBScreen);
    
    uint32_t value_mask, value_list[32];

    m_xcbWindow = xcb_generate_id(m_pXCBConn);

    value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    value_list[0] = m_pXCBScreen->black_pixel;
    value_list[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE |
                    XCB_EVENT_MASK_STRUCTURE_NOTIFY;

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
                      value_mask, 
                      value_list);

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(m_pXCBConn, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* reply =  xcb_intern_atom_reply(m_pXCBConn, cookie, 0);

    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(m_pXCBConn, 0, 16, "WM_DELETE_WINDOW");
    m_pXCBDelWin = xcb_intern_atom_reply(m_pXCBConn, cookie2, 0);

    xcb_change_property(m_pXCBConn, 
                        XCB_PROP_MODE_REPLACE, 
                        m_xcbWindow,
                        (*reply).atom, 
                        4, 
                        32, 
                        1,
                        &(m_pXCBDelWin->atom));
    free(reply);

    xcb_map_window(m_pXCBConn, m_xcbWindow);    
    
    printf("Window %x created\n", m_xcbWindow);

    return true;
}


VkSurfaceKHR XCBControl::CreateSurface(VkInstance& inst)
{
    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.connection = m_pXCBConn;
    surfaceCreateInfo.window = m_xcbWindow;
    
    VkSurfaceKHR surface;
    
    VkResult res = vkCreateXcbSurfaceKHR(inst, &surfaceCreateInfo, NULL, &surface);
    
    if (res != VK_SUCCESS) {
        printf("Error creating surface\n");
        return NULL;
    }
    
    return surface;
}


void XCBControl::PreRun()
{
    xcb_flush(m_pXCBConn);
}


bool XCBControl::PollEvent()
{
//    xcb_generic_event_t* pEvent = xcb_poll_for_event(m_pXCBConn);
    return true;
}