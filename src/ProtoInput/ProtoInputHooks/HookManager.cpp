#include "HookManager.h"
#include "RegisterRawInputHook.h"
#include "GetRawInputDataHook.h"
#include "MessageFilterHook.h"
#include "FocusHook.h"
#include "GetCursorPosHook.h"
#include "SetCursorPosHook.h"
#include "GetKeyStateHook.h"
#include "GetAsyncKeyStateHook.h"
#include "GetKeyboardStateHook.h"

namespace Proto
{

HookManager HookManager::hookManagerInstance{};

HookManager::HookManager()
{
	AddHook<RegisterRawInputHook>(ProtoHookIDs::RegisterRawInputHookID);
	AddHook<GetRawInputDataHook>(ProtoHookIDs::GetRawInputDataHookID);
	AddHook<MessageFilterHook>(ProtoHookIDs::MessageFilterHookID);
	AddHook<FocusHook>(ProtoHookIDs::FocusHooksHookID);
	AddHook<GetCursorPosHook>(ProtoHookIDs::GetCursorPosHookID);
	AddHook<SetCursorPosHook>(ProtoHookIDs::SetCursorPosHookID);
	AddHook<GetKeyStateHook>(ProtoHookIDs::GetKeyStateHookID);
	AddHook<GetAsyncKeyStateHook>(ProtoHookIDs::GetAsyncKeyStateHookID);
	AddHook<GetKeyboardStateHook>(ProtoHookIDs::GetKeyboardStateHookID);
}

void HookManager::InstallHook(ProtoHookIDs hookID)
{
	if (hookID < 0 || hookID >= hookManagerInstance.hooks.size())
		std::cerr << "Trying to install hook ID " << hookID << " which is out of range" << std::endl;
	else
	{
		hookManagerInstance.hooks[hookID]->Install();
	}
}

void HookManager::UninstallHook(ProtoHookIDs hookID)
{
	if (hookID < 0 || hookID >= hookManagerInstance.hooks.size())
		std::cerr << "Trying to uninstall hook ID " << hookID << " which is out of range" << std::endl;
	else
	{
		hookManagerInstance.hooks[hookID]->Uninstall();
	}
}

bool HookManager::IsInstalled(ProtoHookIDs hookID)
{
	if (hookID < 0 || hookID >= hookManagerInstance.hooks.size())
	{
		std::cerr << "Trying to check hook ID " << hookID << " which is out of range" << std::endl;
		return false;
	}
	else
	{
		return hookManagerInstance.hooks[hookID]->IsInstalled();
	}
}

}
