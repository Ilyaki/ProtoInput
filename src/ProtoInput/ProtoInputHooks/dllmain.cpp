// dllmain.cpp : Defines the entry point for the DLL application.
#include <iostream>
#include <BlackBone/Process/Process.h>
#include <BlackBone/Patterns/PatternSearch.h>
#include <BlackBone/Process/RPC/RemoteFunction.hpp>
#include <BlackBone/Syscalls/Syscall.h>
#include <BlackBone/LocalHook/LocalHook.hpp>
// #include <imgui/imgui.h>

#include <GL/glew.h>

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_opengl3.h>

#include <easyhook.h>

#include "TestHooks.h"

#include "Gui.h"

DWORD WINAPI StartThread(LPVOID lpParameter)
{	
    AllocConsole();
    FILE* f = new FILE();
    freopen_s(&f, "CONOUT$", "w", stdout);
    
    std::cout << "Hooks DLL loaded" << std::endl;

    //MessageBoxW(NULL, L"Press OK to start", L"", MB_OK);
	
    TestHooks();

    ShowGui();

    std::cout << "Reached end of startup/GUI thread" << std::endl;
	
    return 0;
}

 extern "C" __declspec(dllexport) void Blah()
{
    std::cout << "blah" << std::endl;
    StartThread(nullptr);
}

EASYHOOK_BOOL_EXPORT EasyHookDllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    EasyHookDllMain(hModule, ul_reason_for_call, lpReserved);
	
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
         //HANDLE hThread = CreateThread(nullptr, 0,
                                     // (LPTHREAD_START_ROUTINE)StartThread, hModule, 0, 0);
         //if (hThread != nullptr)
         //    CloseHandle(hThread);



         //auto blah = new std::thread(StartThread, nullptr);
    		
    		
        // StartThread(nullptr);
    		
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

