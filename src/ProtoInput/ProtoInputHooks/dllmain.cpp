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

// #include "imgui.h"

int MessageBoxW_Hook(
    HWND& hWnd,
    LPCWSTR& lpText,
    LPCWSTR& lpCaption,
    UINT& uType)
{
    std::cout << "Hooked!" << std::endl;

    lpText = L"hooked";
    lpCaption = L"hooked!";

    return 0;
}


/*
HWND hGameWindow;
// WNDPROC hGameWindowProc;
bool menuShown = true;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// LRESULT CALLBACK windowProc_hook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
// {
//     // Toggle the overlay using the delete key
//     if (uMsg == WM_KEYDOWN && wParam == VK_DELETE) {
//         menuShown = !menuShown;
//         return false;
//     }
//
//     // If the overlay is shown, direct input to the overlay only
//     if (menuShown) 
//     {
//     	//FIXME
//         //CallWindowProc(ImGui_ImplWin32_WndProcHandler, hWnd, uMsg, wParam, lParam);
//         return true;
//     }
//
//     // Otherwise call the game's wndProc function
//     return CallWindowProc(hGameWindowProc, hWnd, uMsg, wParam, lParam);
// }

void glSwapBuffers_hook(HDC& hDc)
{
    // MessageBoxA(NULL, "Hello", "Hello", MB_OK);
    std::cout << "swap" << std::endl;
	   //
    // Initialize glew and imgui but only once
    static bool imGuiInitialized = false;
    if (!imGuiInitialized) {
        imGuiInitialized = true;
    
        // Get the game's window from it's handle
        hGameWindow = WindowFromDC(hDc);
        std::cout << "hGameWindow = " << hGameWindow << std::endl;
    
        // Overwrite the game's wndProc function
        // hGameWindowProc = (WNDPROC)SetWindowLongPtr(hGameWindow,
        //                                             GWLP_WNDPROC, (LONG_PTR)windowProc_hook);
    
        // Init glew, create imgui context, init imgui
		 glewInit();
    ImGui::CreateContext();
    	
		
    	//gives access violation
	 ImGui_ImplWin32_Init(hGameWindow);


    	
    // ImGui_ImplOpenGL3_Init();
    // ImGui::StyleColorsDark();
    //     ImGui::GetStyle().AntiAliasedFill = false;
    //     ImGui::GetStyle().AntiAliasedLines = false;
    //     ImGui::CaptureMouseFromApp();
    //     ImGui::GetStyle().WindowTitleAlign = ImVec2(0.5f, 0.5f);
    //     std::cout << "Completed imgui init" << std::endl;
    }
	
    //
    // //TODO; toggle with getasynckeystate or something
	   //
    // // If the menu is shown, start a new frame and draw the demo window
    // if (menuShown) {
    //     ImGui_ImplOpenGL3_NewFrame();
    //     ImGui_ImplWin32_NewFrame();
    //     ImGui::NewFrame();
    //     ImGui::ShowDemoWindow();
    //     ImGui::Render();
    //
    //     // Draw the overlay
    //     ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // }
    

    // return true;
}

BOOL SwapBuffers_Hook(HDC& hdc)
{
    std::cout << "SwapBuffers" << std::endl;
    return TRUE;
}
*/

DWORD WINAPI StartThread(LPVOID lpParameter)
{	
    AllocConsole();
    FILE* f = new FILE();
    freopen_s(&f, "CONOUT$", "w", stdout);
    
    std::cout << "Hooks DLL loaded" << std::endl;

    TestHooks();
	
    // {
    //     blackbone::Detour<decltype(&MessageBoxW)> detour{};
    //
    //     detour.Hook(
    //         reinterpret_cast<blackbone::Detour<decltype(&MessageBoxW)>::type>(GetProcAddress(GetModuleHandleW(L"user32.dll"), "MessageBoxW")),
    //         &MessageBoxW_Hook,
    //         blackbone::HookType::Inline,
    //         blackbone::CallOrder::HookFirst,
    //         blackbone::ReturnMethod::UseOriginal
    //     );
    //
    //     MessageBoxW(NULL, L"hello", L"world", MB_OK);
    // }

    // {
    //     blackbone::Detour<void(*)(HDC__*)> detour{};
    //         	
    //     detour.Hook(//TODO: might be "wglSwapBuffers"
    //         reinterpret_cast<void(*)(HDC__*)>(GetProcAddress(GetModuleHandleW(L"opengl32.dll"), "glSwapBuffers")),
    //         &glSwapBuffers_hook,
    //         blackbone::HookType::Inline,
    //         blackbone::CallOrder::HookLast,
    //         blackbone::ReturnMethod::UseOriginal
    //     );
    // }

	// {
 //        blackbone::Detour<void(*)(HDC__*)> detour{};
 //
 //        //auto proc = GetProcAddress(GetModuleHandleW(L"gdi32full.dll"), "SwapBuffers");
	// 	auto proc = GetProcAddress(GetModuleHandleW(L"opengl32.dll"), "wglSwapBuffers");
 //        // auto proc = GetProcAddress(GetModuleHandleW(L"gdi32full.dll"), "NtGdiSwapBuffers");
 //        // auto proc = GetProcAddress(GetModuleHandleW(L"gdi32.dll"), "GdiSwapBuffers");
 //        // auto proc = GetProcAddress(GetModuleHandleW(L"gdi32.dll"), "SwapBuffers");
 //    	
 //        std::cout << "proc = " << proc << std::endl;
 //    	
 //        bool res = detour.Hook(//TODO: might be "SwapBuffers"
 //            reinterpret_cast<void(*)(HDC__*)>(proc),
 //            &glSwapBuffers_hook,
 //            blackbone::HookType::HWBP,
 //            blackbone::CallOrder::HookLast,
 //            blackbone::ReturnMethod::UseOriginal
 //        );
 //
 //        std::cout << "hook success = " << res << std::endl;
 //    }

    // {
    //     blackbone::Detour<decltype(&SwapBuffers)> detour{};
    //
    //     //auto proc = GetProcAddress(GetModuleHandleW(L"gdi32full.dll"), "SwapBuffers");
    //     // auto proc = GetProcAddress(GetModuleHandleW(L"opengl32.dll"), "wglSwapBuffers");
    //     // auto proc = GetProcAddress(GetModuleHandleW(L"gdi32full.dll"), "NtGdiSwapBuffers");
    //     // auto proc = GetProcAddress(GetModuleHandleW(L"gdi32.dll"), "GdiSwapBuffers");
    //     auto proc = GetProcAddress(GetModuleHandleW(L"gdi32.dll"), "SwapBuffers");
    //
    //     std::cout << "proc = " << proc << std::endl;
    //
    //     bool res = detour.Hook(//TODO: might be "SwapBuffers"
    //                            reinterpret_cast<blackbone::Detour<decltype(&SwapBuffers)>::type>(proc),
    //                            (blackbone::Detour<decltype(&SwapBuffers)>::hktype )&SwapBuffers_Hook,
    //                            blackbone::HookType::HWBP,
    //                            blackbone::CallOrder::NoOriginal,
    //                            blackbone::ReturnMethod::UseNew
    //     );
    //
    //     std::cout << "hook success = " << res << std::endl;
    // }
	
    return 0;
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
        HANDLE hThread = CreateThread(nullptr, 0,
                                      (LPTHREAD_START_ROUTINE)StartThread, hModule, 0, 0);
        if (hThread != nullptr)
            CloseHandle(hThread);

        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

