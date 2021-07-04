#pragma once
#include "Hook.h"
#include "InstallHooks.h"

namespace Proto
{

class DinputOrderHook final : public Hook
{
private:
	std::vector<HookInfo> hookInfos{};
	
public:
	const char* GetHookName() const override { return "Dinput Order"; }
	const char* GetHookDescription() const override
	{
		return
			"Hooks DinputDeviceCreate, which is typically called at startup to create an interface used to get controller input. "
			"The hook will always used the selected controller GUID. This should be passed to the API at startup to guarantee that the game will use it.";
	}
	bool HasGuiStatus() const override { return true; }
	void ShowGuiStatus() override;
	void InstallImpl() override;
	void UninstallImpl() override;

	static void SetControllerGuid(const GUID& guid);
};

}
