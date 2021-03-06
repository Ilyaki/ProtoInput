#include "TestHooks.h"
#include <iostream>
#include <easyhook.h>
#include <GL/glew.h>
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_opengl3.h>

HWND hGameWindow;
WNDPROC hGameWindowProc;
bool menuShown = false;

typedef BOOL(WINAPI* wglSwapBuffers_t) (_In_ HDC hDc);
wglSwapBuffers_t originalFunc = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI ImGui_ImplWin32_WndProcHandle_Wrapper(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

BOOL FilterMessage(const LPMSG lpMsg)
{
	const auto msg = lpMsg->message;
	const auto wParam = lpMsg->wParam;
	const auto lParam = lpMsg->lParam;
	const auto hwnd = lpMsg->hwnd;

#define ALLOW return 1
	//Massive performance benefits for returning a successful WM_NULL compared to causing an error in the application.
#define BLOCK memset(lpMsg, 0, sizeof(MSG)); return 1


	if (static bool init = false; !init)
	{
		init = true;
		std::cout << "FilterMessage loaded" << std::endl;
	}

	// Toggle the overlay using the delete key
	// if (uMsg == WM_KEYDOWN && wParam == VK_DELETE) 
	// {
	//     menuShown = !menuShown;
	//     std::cout << "Menu shown = " << menuShown << std::endl;
	//     return false;
	// }

	// If the overlay is shown, direct input to the overlay only
	if (menuShown)
	{
		//CallWindowProc(ImGui_ImplWin32_WndProcHandle_Wrapper, hwnd, msg, wParam, lParam);
		ImGui_ImplWin32_WndProcHandle_Wrapper(hwnd, msg, wParam, lParam);
		
		if ((msg >= WM_KEYFIRST && msg <= WM_KEYLAST) || (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST))
			BLOCK;
	}

	ALLOW;

#undef ALLOW
#undef BLOCK
}

BOOL WINAPI GetMessageA_Hook(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
	const auto ret = GetMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);

	return ret == -1 ? -1 : FilterMessage(lpMsg);
}

BOOL WINAPI GetMessageW_Hook(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
	const auto ret = GetMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);

	return ret == -1 ? -1 : FilterMessage(lpMsg);
}

BOOL WINAPI PeekMessageA_Hook(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	const auto ret = PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

	return ret == FALSE ? FALSE : ((1 + FilterMessage(lpMsg)) / 2);//TODO: filterMessage returns -1 but PeekMessage expects FALSE(0): is this okay?
}

BOOL WINAPI PeekMessageW_Hook(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	const auto ret = PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

	return ret == FALSE ? FALSE : ((1 + FilterMessage(lpMsg)) / 2);
}

NTSTATUS InstallHook(void* address, void* callback)
{
	// Perform hooking
	HOOK_TRACE_INFO hHook = { NULL }; // keep track of our hook

	// Install the hook
	const auto result = LhInstallHook(
		address,
		callback,
		NULL,
		&hHook);

	if (FAILED(result))
	{
		std::cerr << "Error installing hook, error string: " << RtlGetLastErrorString() << std::endl;
	}
	else
	{
		// If the threadId in the ACL is set to 0,
		// then internally EasyHook uses GetCurrentThreadId()
		ULONG ACLEntries[1] = { 0 };

		// Disable the hook for the provided threadIds, enable for all others
		const auto ACLres = LhSetExclusiveACL(ACLEntries, 1, &hHook);

		if (FAILED(ACLres))
		{
			std::cerr << "Error setting ACL, result = " << ACLres << ", error string: " << RtlGetLastErrorString() << std::endl;
		}
		else
		{
			std::cout << "Successfully installed hook, result: " << result << std::endl;
		}
	}

	return result;
}

NTSTATUS InstallHook(const LPCWSTR moduleHandle, const LPCSTR proc, void* callBack, const bool isOrdinal = false)
{
	// Perform hooking
	HOOK_TRACE_INFO hHook = { NULL }; // keep track of our hook

	// Install the hook
	const auto result = LhInstallHook(
		static_cast<void*>(GetProcAddress(GetModuleHandleW(moduleHandle), proc)),
		callBack,
		NULL,
		&hHook);

	// Don't treat proc as an actual pointer to a string if it used as an ordinal.
	const auto name = isOrdinal ? "ORDINAL" : proc;

	if (FAILED(result))
	{
		std::cerr << "Error installing " << name << " hook, error string: " << RtlGetLastErrorString() << std::endl;
	}
	else
	{
		// If the threadId in the ACL is set to 0,
		// then internally EasyHook uses GetCurrentThreadId()
		ULONG ACLEntries[1] = { 0 };

		// Disable the hook for the provided threadIds, enable for all others
		const auto ACLres = LhSetExclusiveACL(ACLEntries, 1, &hHook);

		if (FAILED(ACLres))
		{
			std::cerr << "Error setting ACL for " << name << " hook, ACLres = " << ACLres << ", error string: " << RtlGetLastErrorString() << std::endl;
		}
		else
		{
			std::cout << "Successfully installed " << name << " hook, in module: " << moduleHandle << ", result: " << result << std::endl;
		}
	}

	return result;
}

LRESULT CALLBACK WindowProc_Hook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (static bool init = false; !init)
    {
        init = true;
        std::cout << "WndProc loaded" << std::endl;
    }

    // Toggle the overlay using the delete key
    // if (uMsg == WM_KEYDOWN && wParam == VK_DELETE) 
    // {
    //     menuShown = !menuShown;
    //     std::cout << "Menu shown = " << menuShown << std::endl;
    //     return false;
    // }

    // If the overlay is shown, direct input to the overlay only
    if (menuShown) 
    {
        CallWindowProc(ImGui_ImplWin32_WndProcHandle_Wrapper, hWnd, uMsg, wParam, lParam);

    	if ((uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST) || (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST))
			return true;
    }

    // Otherwise call the game's wndProc function
    return CallWindowProc(hGameWindowProc, hWnd, uMsg, wParam, lParam);
}

LRESULT WINAPI WndProcHook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// bool callOriginal = true;
	//
	// std::cout << "WndProcHook" << std::endl;
	//
	// return callOriginal ? hGameWindowProc(hWnd, uMsg, wParam, lParam) : TRUE;

	return TRUE;
}

BOOL WINAPI glSwapBuffers_hook(HDC hdc)
{
    // Initialize glew and imgui but only once
    if (static bool imGuiInitialized = false;  !imGuiInitialized) {
        imGuiInitialized = true;

        // Get the game's window from it's handle
        hGameWindow = WindowFromDC(hdc);
        std::cout << "hGameWindow = " << hGameWindow << std::endl;

        // Overwrite the game's wndProc function
        // hGameWindowProc = (WNDPROC)SetWindowLongPtr(hGameWindow,
        //                                             GWLP_WNDPROC, (LONG_PTR)WindowProc_Hook);


    	
		// hGameWindowProc = (WNDPROC)GetWindowLongW(hGameWindow,GWLP_WNDPROC);
		// std::cout << "hGameWindowProc = " << hGameWindowProc << std::endl;
		// InstallHook(hGameWindowProc, &WndProcHook);
		// std::cout << "Installed hGameWindowProc hook" << std::endl;
    	
        // Init glew, create imgui context, init imgui
        glewInit();
        ImGui::CreateContext();


        ImGui_ImplWin32_Init(hGameWindow);

        ImGui_ImplOpenGL3_Init();
        ImGui::StyleColorsDark();
        ImGui::GetStyle().AntiAliasedFill = false;
        ImGui::GetStyle().AntiAliasedLines = false;
        ImGui::CaptureMouseFromApp();
        ImGui::GetStyle().WindowTitleAlign = ImVec2(0.5f, 0.5f);
        std::cout << "Completed imgui init" << std::endl;
    }

    if (GetAsyncKeyState(VK_HOME))
    {
        menuShown = !menuShown;
	    std::cout << "menu shown = " << menuShown << std::endl;
	}
	
    // If the menu is shown, start a new frame and draw the demo window
    if (menuShown) {
        ImGui_ImplOpenGL3_NewFrame(); 
        ImGui_ImplWin32_NewFrame(); 
        ImGui::NewFrame();
    	
        ImGui::ShowDemoWindow();
    	
        // ImGui::Begin("Hello");
        // ImGui::Button("test");
        // ImGui::End();
    	
        ImGui::Render();
    
        // Draw the overlay
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		menuShown = true;
    }
	
    return originalFunc(hdc);
}


void TestHooks()
{
	originalFunc = reinterpret_cast<wglSwapBuffers_t>(GetProcAddress(GetModuleHandleW(L"opengl32.dll"), "wglSwapBuffers"));
	std::cout << "original function ptr = " << originalFunc << std::endl;
	
	InstallHook(L"user32", "GetMessageA", GetMessageA_Hook);
	InstallHook(L"user32", "GetMessageW", GetMessageW_Hook);
	InstallHook(L"user32", "PeekMessageA", PeekMessageA_Hook);
	InstallHook(L"user32", "PeekMessageW", PeekMessageW_Hook);
		
	InstallHook(L"opengl32.dll", "wglSwapBuffers", &glSwapBuffers_hook);
}
