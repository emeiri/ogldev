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

#ifndef VULKAN_WIN32_CONTROL_H
#define VULKAN_WIN32_CONTROL_H

#ifdef WIN32

#include "ogldev_vulkan.h"

class Win32Control : public VulkanWindowControl
{
public:
    Win32Control(const char* pAppName);
    
    ~Win32Control();
    
    virtual void Init(uint Width, uint Height);
    
    virtual VkSurfaceKHR CreateSurface(VkInstance& inst);

 private:    

    HINSTANCE   m_hinstance;
    HWND        m_hwnd;
    std::wstring m_appName;
};

#endif

#endif