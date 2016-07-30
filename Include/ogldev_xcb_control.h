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

#ifndef VULKAN_XCB_CONTROL_H
#define VULKAN_XCB_CONTROL_H

#ifndef WIN32

#include <xcb/xcb.h>

class XCBControl : public VulkanWindowControl
{
public:
    XCBControl();
    
    ~XCBControl();
    
    virtual bool Init(uint Width, uint Height);
    
    virtual kSurfaceKHR CreateSurface();

    virtual void PreRun();

    virtual bool PollEvent() = 0;

    xcb_connection_t* m_pXCBConn;
    xcb_screen_t* m_pXCBScreen;
    xcb_window_t m_xcbWindow;           
    xcb_intern_atom_reply_t* m_pXCBDelWin;    
};

#endif

#endif