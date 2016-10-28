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


#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
#else
int main(int argc, char** argv)
#endif
{
    VulkanWindowControl* pWindowControl = NULL;
#ifdef WIN32
    pWindowControl = new Win32Control(hInstance, pAppName);
#else            
    pWindowControl = new XCBControl();
#endif    
    pWindowControl->Init(WINDOW_WIDTH, WINDOW_HEIGHT);

    OgldevVulkanCore core(pAppName);
    core.Init(pWindowControl);
    
    return 0;
}


#if 0
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

            EndPaint(hwnd, &ps);
        }
        return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{

    WNDCLASSEX wndcls = {};

    wndcls.cbSize = sizeof(wndcls);
    //wndcls.style = CS_HREDRAW | CS_VREDRAW;
    wndcls.lpfnWndProc = WindowProc;
    wndcls.hInstance = hInstance; 
    wndcls.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndcls.lpszClassName = L"ogldev";

    if (!RegisterClassEx(&wndcls)) {
        DWORD error = GetLastError();
        OGLDEV_ERROR("RegisterClassEx error %d", error);
    }


    HWND hwnd = CreateWindowEx(0,
                          L"ogldev",                  // class name
                            L"ogldev",//NULL,//(LPCWSTR)m_pAppName,        // app name
                            WS_OVERLAPPEDWINDOW,// |       // window style
                           // WS_VISIBLE | WS_SYSMENU,
                       //     100, 100,                   // window start
                        //    Width, 
                        //    Height, 
                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                            NULL,          
                            NULL,          
                            hInstance,   
                            NULL);  
    /*HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        L"ogldev",                     // Window class
        L"Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );*/

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
    }
#endif