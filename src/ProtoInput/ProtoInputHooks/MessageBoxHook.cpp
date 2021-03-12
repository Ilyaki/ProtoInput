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
	auto [status, _hookInfo] = InstallNamedHook(L"user32", "MessageBoxW", Hook_MessageBoxW);
	this->hookInfo = _hookInfo;
}

void MessageBoxHook::UninstallImpl()
{
	UninstallHook(&hookInfo);
}

}