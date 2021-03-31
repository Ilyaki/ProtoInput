#include "ClipCursorHook.h"

namespace Proto
{

BOOL WINAPI Hook_ClipCursor(const RECT* lpRect)
{
	return TRUE;
}

void ClipCursorHook::InstallImpl()
{
	hookInfo = std::get<1>(InstallNamedHook(L"user32", "ClipCursor", Hook_ClipCursor));
}

void ClipCursorHook::UninstallImpl()
{
	UninstallHook(&hookInfo);
}

}
