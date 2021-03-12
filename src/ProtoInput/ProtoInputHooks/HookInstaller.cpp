#include <iostream>
#include <easyhook.h>
#include <imgui.h>
#include "InstallHooks.h"
#include "Gui.h"
#include <vector>

namespace Proto
{

//TODO: This is going to have problems with the EasyHook thread ACL since we are using multiple threads

static HWND protoHwnd;

std::vector<unsigned long> ACLThreads{};
std::vector<HOOK_TRACE_INFO> trackedHooks{};

void RefreshACL()
{
	for (auto& hook : trackedHooks)
	{
		const auto ACLres = LhSetExclusiveACL(&ACLThreads[0], ACLThreads.size(), &hook);

		// FAILED(ACLres) ?
		// 	printf("Error setting ACL, res = 0x%X, error string = %ls\n", ACLres, RtlGetLastErrorString()) :
		// 	printf("Successfully set ACL\n");

		// for (int i = 0; i < ACLThreads.size(); i++)
		// 	printf("  Using ACL thread %d\n", ACLThreads[i]);
	}
}

void AddThreadToACL(unsigned long threadID)
{
	printf("Adding thread ID %d to ACL\n", threadID);
	ACLThreads.push_back(threadID);
	RefreshACL();
}

void AddTrackedHook(HOOK_TRACE_INFO hook)
{
	trackedHooks.push_back(hook);
	RefreshACL();
}

void RemoveTrackedHook(HOOK_TRACE_INFO hook)
{
	// We don't really need to do anything, since EasyHook will just give a graceful error when setting ACL of an ejected hook.
	
	//RefreshACL();
}

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
		// ULONG ACLEntries[2] = { 0, Proto::GuiThreadID };
		//
		// // Disable the hook for the provided threadIds, enable for all others
		// const auto ACLres = LhSetExclusiveACL(ACLEntries, 2, &hHook);
		//
		// if (FAILED(ACLres))
		// {
		// 	std::cerr << "Error setting ACL, result = " << ACLres << ", error string: " << RtlGetLastErrorString() << std::endl;
		// }
		// else
		// {
		// 	std::cout << "Successfully installed hook, result: " << result << std::endl;
		// }
		
		AddTrackedHook(hHook);
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
		// // If the threadId in the ACL is set to 0,
		// // then internally EasyHook uses GetCurrentThreadId()
		// ULONG ACLEntries[2] = { 0, Proto::GuiThreadID };
		//
		// // Disable the hook for the provided threadIds, enable for all others
		// const auto ACLres = LhSetExclusiveACL(ACLEntries, 2, &hHook);
		//
		// if (FAILED(ACLres))
		// {
		// 	std::cerr << "Error setting ACL for " << name << " hook, ACLres = " << ACLres << ", error string: " << RtlGetLastErrorString() << std::endl;
		// }
		// else
		// {
		// 	std::cout << "Successfully installed " << name << " hook, in module: " << moduleHandle << ", result: " << result << std::endl;
		// }
		
		AddTrackedHook(hHook);
	}

	return { result, {!(FAILED(result)), hHook} };
}

void UninstallHook(HookInfo* hookInfo)
{
	if (hookInfo->hasHook)
	{
		std::cout << "Uninstalling an EasyHook hook" << std::endl;
		LhUninstallHook(&hookInfo->info);
		RemoveTrackedHook(hookInfo->info);
		hookInfo->hasHook = false;
	}
}

}
