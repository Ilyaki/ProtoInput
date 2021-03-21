#include "MessageBoxHook.h"
#include "InstallHooks.h"
#include <imgui.h>

namespace Proto
{

int WINAPI Hook_MessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	std::cout << "Intercepted MessageBoxW call, current thread = " << GetCurrentThreadId() << std::endl;
	return 0;
}

void MessageBoxHook::ShowGuiStatus()
{

}

void MessageBoxHook::InstallImpl()
{
	hookInfo = std::get<1>(InstallNamedHook(L"user32", "MessageBoxW", Hook_MessageBoxW));
}

void MessageBoxHook::UninstallImpl()
{
	UninstallHook(&hookInfo);
}

}