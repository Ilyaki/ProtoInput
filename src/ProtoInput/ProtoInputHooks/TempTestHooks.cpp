#include <iostream>
#include <easyhook.h>
#include <GL/glew.h>
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_opengl3.h>

namespace Proto
{

HWND hGameWindow;
WNDPROC hGameWindowProc;
bool menuShown = false;

typedef BOOL(WINAPI* wglSwapBuffers_t) (_In_ HDC hDc);
wglSwapBuffers_t originalFunc = nullptr;

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

void TestHooks()
{
	// InstallHook(L"user32", "GetMessageA", GetMessageA_Hook);
	// InstallHook(L"user32", "GetMessageW", GetMessageW_Hook);
	// InstallHook(L"user32", "PeekMessageA", PeekMessageA_Hook);
	// InstallHook(L"user32", "PeekMessageW", PeekMessageW_Hook);
}

}