#pragma once
#include "Hook.h"
#include "HookManager.h"
#include <bitset>
#include <hidusage.h>

namespace Proto
{

class RegisterRawInputHook final : public Hook
{
private:
	//TODO: add to gui and/or pipe (or just set the bitfield from the pipe).
	bool updateDeviceTypeOnGameUpdate = true;
	
	bool installedAtLeastOnce = false;
	
	HookInfo hookInfo{};
		
	void FindAlreadySubscribedWindows();

public:
	const char* GetHookName() const override { return "Register Raw Input"; }
	const char* GetHookDescription() const override
	{
		return
		"Disconnects the game from raw input and subscribes our window instead.\n"
		"We subscribe to raw input in the foreground and background so we always receive input.\n"
		"Forwards raw input back to the original window.\n"
		"Disabling this hook after it's installed may not fully uninstall it.\n";
	}

	bool HasGuiStatus() const override { return true; }
	void ShowGuiStatus() override;
	void InstallImpl() override;
	void UninstallImpl() override;

	void AddWindowToForward(HWND hwnd, std::bitset<9> usages);
		
	RegisterRawInputHook();
};

}
