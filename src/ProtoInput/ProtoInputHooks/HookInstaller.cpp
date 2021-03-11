#include <iostream>
#include <easyhook.h>
#include <imgui.h>
#include "InstallHooks.h"
#include "Gui.h"

namespace Proto
{

//TODO: This is going to have problems with the EasyHook thread ACL since we are using multiple threads

static HWND protoHwnd;

std::tuple<NTSTATUS, HookInfo> InstallHook(void* address, void* callback)
{
	// Perform hooking
	HOOK_TRACE_INFO hHook = { NULL }; // keep track of our hook

	// Install the hook
	const auto result = LhInstallHook(
		address,
		callback,
		NULL,
		&hHook);

	if (FAILED(result))
	{
		std::cerr << "Error installing hook, error string: " << RtlGetLastErrorString() << std::endl;
	}
	else
	{
		// If the threadId in the ACL is set to 0,
		// then internally EasyHook uses GetCurrentThreadId()
		//TODO: could add the named pipe thread to the ACL list, just in case
		ULONG ACLEntries[2] = { 0, Proto::GuiThreadID };

		// Disable the hook for the provided threadIds, enable for all others
		const auto ACLres = LhSetExclusiveACL(ACLEntries, 2, &hHook);

		if (FAILED(ACLres))
		{
			std::cerr << "Error setting ACL, result = " << ACLres << ", error string: " << RtlGetLastErrorString() << std::endl;
		}
		else
		{
			std::cout << "Successfully installed hook, result: " << result << std::endl;
		}
	}

	return { result, {!(FAILED(result)), hHook} };
}

std::tuple<NTSTATUS, HookInfo> InstallNamedHook(const LPCWSTR moduleHandle, const LPCSTR proc, void* callBack, const bool isOrdinal)
{
	// Perform hooking
	HOOK_TRACE_INFO hHook = { NULL }; // keep track of our hook
		
	// Install the hook
	const auto result = LhInstallHook(
		static_cast<void*>(GetProcAddress(GetModuleHandleW(moduleHandle), proc)),
		callBack,
		NULL,
		&hHook);

	// Don't treat proc as an actual pointer to a string if it used as an ordinal.
	const auto name = isOrdinal ? "ORDINAL" : proc;

	if (FAILED(result))
	{
		std::cerr << "Error installing " << name << " hook, error string: " << RtlGetLastErrorString() << std::endl;
	}
	else
	{
		// If the threadId in the ACL is set to 0,
		// then internally EasyHook uses GetCurrentThreadId()
		ULONG ACLEntries[2] = { 0, Proto::GuiThreadID };

		// Disable the hook for the provided threadIds, enable for all others
		const auto ACLres = LhSetExclusiveACL(ACLEntries, 2, &hHook);

		if (FAILED(ACLres))
		{
			std::cerr << "Error setting ACL for " << name << " hook, ACLres = " << ACLres << ", error string: " << RtlGetLastErrorString() << std::endl;
		}
		else
		{
			std::cout << "Successfully installed " << name << " hook, in module: " << moduleHandle << ", result: " << result << std::endl;
		}
	}

	return { result, {!(FAILED(result)), hHook} };
}

void UninstallHook(HookInfo* hookInfo)
{
	if (hookInfo->hasHook)
	{
		std::cout << "Uninstalling an EasyHook hook" << std::endl;
		LhUninstallHook(&hookInfo->info);
		hookInfo->hasHook = false;
	}
}

}
