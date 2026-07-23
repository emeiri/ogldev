#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

void __declspec(dllexport) export_test()
{

}

// dllmain.cpp : Defines the entry point for the DLL application.
BOOL APIENTRY DllMain(HMODULE Module, DWORD reason_for_call, LPVOID pReserved)
{
    switch (reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

