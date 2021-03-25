#include "CursorVisibilityHook.h"
#include "FakeCursor.h"

namespace Proto
{

int WINAPI Hook_ShowCursor(BOOL bShow)
{
	FakeCursor::SetCursorVisibility(bShow == TRUE);
	if (bShow == FALSE) ShowCursor(FALSE);
	return bShow == TRUE ? 0 : -1;
}

HCURSOR WINAPI Hook_SetCursor(HCURSOR hCursor)
{
	FakeCursor::SetCursorVisibility(hCursor != nullptr);

	if (hCursor != nullptr)
		FakeCursor::SetCursorHandle(hCursor); // Custom cursor image
	
	return hCursor;
}

void CursorVisibilityHook::InstallImpl()
{
	hookInfoShowCursor = std::get<1>(InstallNamedHook(L"user32", "ShowCursor", Hook_ShowCursor));
	hookInfoSetCursor = std::get<1>(InstallNamedHook(L"user32", "SetCursor", Hook_SetCursor));
}

void CursorVisibilityHook::UninstallImpl()
{
	UninstallHook(&hookInfoSetCursor);
	UninstallHook(&hookInfoShowCursor);
}

}
