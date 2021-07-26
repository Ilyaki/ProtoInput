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

BOOL WINAPI Hook_SetSystemCursor(HCURSOR hCursor, DWORD id)
{
	if (id == 32512) // OCR_NORMAL: Standard arrow
	{
		FakeCursor::SetCursorVisibility(hCursor != nullptr);

		if (hCursor != nullptr)
			FakeCursor::SetCursorHandle(hCursor); // Custom cursor image
	}

	return TRUE;
}

void CursorVisibilityHook::InstallImpl()
{
	hookInfoShowCursor = std::get<1>(InstallNamedHook(L"user32", "ShowCursor", Hook_ShowCursor));
	hookInfoSetCursor = std::get<1>(InstallNamedHook(L"user32", "SetCursor", Hook_SetCursor));
	hookInfoSetSystemCursor = std::get<1>(InstallNamedHook(L"user32", "SetSystemCursor", Hook_SetSystemCursor));
}

void CursorVisibilityHook::UninstallImpl()
{
	UninstallHook(&hookInfoShowCursor);
	UninstallHook(&hookInfoSetCursor);
	UninstallHook(&hookInfoSetSystemCursor);
}

}
