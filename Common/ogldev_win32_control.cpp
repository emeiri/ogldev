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

#ifdef WIN32

#include "ogldev_vulkan.h"
#include "ogldev_win32_control.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

Win32Control::Win32Control(const char* pAppName) 
{
    m_hinstance = GetModuleHandle(NULL);;
    assert(m_hinstance);
    m_hwnd = 0;
    std::string s(pAppName);
    m_appName = std::wstring(s.begin(), s.end());
}


Win32Control::~Win32Control()
{
}


void Win32Control::Init(uint Width, uint Height)
{
    WNDCLASSEX wndcls = {};

    wndcls.cbSize = sizeof(wndcls);
    wndcls.lpfnWndProc = WindowProc;
    wndcls.hInstance = m_hinstance; 
    wndcls.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndcls.lpszClassName = L"ogldev";

    if (!RegisterClassEx(&wndcls)) {
        DWORD error = GetLastError();
        OGLDEV_ERROR("RegisterClassEx error %d", error);
    }

    m_hwnd = CreateWindowEx(0,
                            L"ogldev",                        // class name
                            m_appName.c_str(),        
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE, // window style
                            100, 100,                         // window start
                            Width, 
                            Height, 
                            NULL,          
                            NULL,          
                            m_hinstance,   
                            NULL);  

    if (m_hwnd == 0) {
        DWORD error = GetLastError();
        OGLDEV_ERROR("CreateWindowEx error %d", error);
    }

    ShowWindow(m_hwnd, SW_SHOW);
}


VkSurfaceKHR Win32Control::CreateSurface(VkInstance& inst)
{
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = m_hinstance;
    surfaceCreateInfo.hwnd = m_hwnd;
    
    VkSurfaceKHR surface;
    
    VkResult res = vkCreateWin32SurfaceKHR(inst, &surfaceCreateInfo, NULL, &surface);
    CHECK_VULKAN_ERROR("vkCreateXcbSurfaceKHR error %d\n", res);
    
    return surface;
}

#endif

#endif