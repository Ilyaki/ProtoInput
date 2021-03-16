#include "Gui.h"
#include "HwndSelector.h"
#include <cstdio>


namespace Proto
{

intptr_t HwndSelector::selectedHwnd = 0;

struct HandleData
{
    unsigned long pid;
    HWND hwnd;
};

BOOL IsMainWindow(HWND handle)
{
	// Is top level & visible & not one of ours
    return
		GetWindow(handle, GW_OWNER) == (HWND)0 && 
        IsWindowVisible(handle) &&
        handle != (HWND)Proto::ConsoleHwnd &&
        handle != Proto::ProtoGuiHwnd;
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
    HandleData& data = *(HandleData*)lParam;
	
    DWORD pid = 0;
    GetWindowThreadProcessId(handle, &pid);
	
    if (data.pid != pid || !IsMainWindow(handle))
        return TRUE; // Keep searching
	
    data.hwnd = handle;
	
    return FALSE;
}

void HwndSelector::UpdateMainHwnd()
{
    // Go through all the top level windows, select the first that's visible & belongs to the process
	
    HandleData data { GetCurrentProcessId(), nullptr };
    EnumWindows(EnumWindowsCallback, (LPARAM)&data);

    const auto hwnd = (intptr_t)data.hwnd;

    if (hwnd == 0)
        printf("Warning: UpdateMainHwnd didn't find a main window\n");
    else
		printf("UpdateMainHwnd found hwnd %d (0x%X)\n", hwnd, hwnd);

    if (data.hwnd != nullptr)
        selectedHwnd = (intptr_t)data.hwnd;
}

}
