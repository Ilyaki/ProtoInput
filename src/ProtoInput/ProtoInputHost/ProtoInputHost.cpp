// ProtoInputHost.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <BlackBone/Process/Process.h>
#include <BlackBone/Patterns/PatternSearch.h>
#include <BlackBone/Process/RPC/RemoteFunction.hpp>
#include <BlackBone/Syscalls/Syscall.h>
#include <BlackBone/LocalHook/LocalHook.hpp>

#include "protoloader.h"
#include <filesystem>

bool CheckBuildTimings(const std::wstring& folderpath)
{
	//TODO: only if debug

	const auto loaderPath32 = std::filesystem::path{ folderpath }.append("ProtoInputLoader32.dll");
	const auto loaderPath64 = std::filesystem::path{ folderpath }.append("ProtoInputLoader64.dll");

	const auto hookPath32 = std::filesystem::path{ folderpath }.append("ProtoInputHooks32.dll");
	const auto hookPath64 = std::filesystem::path{ folderpath }.append("ProtoInputHooks64.dll");
	
	auto secLoader = abs(std::chrono::duration_cast<std::chrono::seconds>(last_write_time(loaderPath32) - last_write_time(loaderPath64))).count();
	auto secHooks = abs(std::chrono::duration_cast<std::chrono::seconds>(last_write_time(hookPath32) - last_write_time(hookPath64))).count();
	
	constexpr int maximumDurationSec = 40;

	if (secLoader > maximumDurationSec || secHooks > maximumDurationSec)
	{
		return IDCANCEL == MessageBoxW(NULL, L"Hooks32/64 or Loader32/64 are built at very different times\nMake sure you have compiled both", L"Warning", MB_OKCANCEL);
	}

	return false;
}

int main()
{
	wchar_t pathchars[MAX_PATH];
	GetModuleFileNameW(NULL, pathchars, MAX_PATH);
	std::wstring folderpath = pathchars;
	size_t pos = folderpath.find_last_of(L"\\");
	if (pos != std::string::npos)
		folderpath = folderpath.substr(0, pos + 1);
	
	std::wcout << L"Folder name = '" << folderpath << "'" <<  std::endl;

	if (CheckBuildTimings(folderpath))
		return 0;
	
	constexpr bool runtime = false;
	constexpr bool hookSelf = true;

	if (runtime)
	{
		DWORD selectedPid = 0;

		if (hookSelf)
			selectedPid = GetCurrentProcessId();
		else
		{
			// auto pids = blackbone::Process::EnumByName(L"osu!.exe");
			auto pids = blackbone::Process::EnumByName(L"Terraria.exe");
			for (const auto& pid : pids)
			{
				std::cout << "Selected pid " << pid << std::endl;

				selectedPid = pid;
				
				// const auto instanceHandle = BlackBoneInjectRuntime(pid, folderpath.c_str());
				// InstallHook(instanceHandle, ProtoHookIDs::MessageBoxHookID);
				// InstallHook(instanceHandle, ProtoHookIDs::RegisterRawInputHookID);
				// InstallHook(instanceHandle, ProtoHookIDs::GetRawInputDataHookID);
			}
		}

		if (selectedPid != 0)
		{
			const auto instanceHandle = BlackBoneInjectRuntime(selectedPid, folderpath.c_str());
			InstallHook(instanceHandle, ProtoHookIDs::MessageBoxHookID);
		}
	}
	else
	{
		auto path = LR"(C:\WINDOWS\system32\notepad.exe)";
		// auto path = LR"(I:\Software\osu\osu!.exe)";
		unsigned long pid;

		ProtoInstanceHandle instanceHandle = EasyHookInjectStartup(
				path, L"", 0, folderpath.c_str(), &pid);
		

		InstallHook(instanceHandle, ProtoHookIDs::MessageBoxHookID);
		InstallHook(instanceHandle, ProtoHookIDs::RegisterRawInputHookID);
		InstallHook(instanceHandle, ProtoHookIDs::GetRawInputDataHookID);
		InstallHook(instanceHandle, ProtoHookIDs::MessageFilterHookID);

		EnableMessageFilter(instanceHandle, ProtoMessageFilterIDs::TestFilterID);

		// EnableMessageBlock(instanceHandle, 0x00FF); // WM_INPUT
		
		WakeUpProcess(instanceHandle);
	}

	// Don't want to end the pipe immediately
	MessageBoxW(NULL, L"Close to exit ProtoInputHost", L"", MB_OK);
}
