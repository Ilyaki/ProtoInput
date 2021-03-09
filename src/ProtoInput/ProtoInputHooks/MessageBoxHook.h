#pragma once
#include "Hook.h"
#include "InstallHooks.h"

namespace Proto
{

class MessageBoxHook final : public Hook
{
private:
	HookInfo hookInfo{};
	
public:
	const char* GetHookName() const override { return "Message Box"; }
	const char* GetHookDescription() const override { return "This hook will prevent the application from calling MessageBoxW"; }
	bool HasGuiStatus() const override { return true; }
	void ShowGuiStatus() override;
	void InstallImpl() override;
	void UninstallImpl() override;
};

}
