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

int main()
{
	wchar_t pathchars[MAX_PATH];
	GetModuleFileNameW(NULL, pathchars, MAX_PATH);
	std::wstring folderpath = pathchars;
	size_t pos = folderpath.find_last_of(L"\\");
	if (pos != std::string::npos)
		folderpath = folderpath.substr(0, pos + 1);
	
	std::wcout << L"Folder name = '" << folderpath << "'" <<  std::endl;


	constexpr bool runtime = false;
	constexpr bool hookSelf = false;

	if (runtime)
	{
		DWORD selectedPid = 0;

		if (hookSelf)
			selectedPid = GetCurrentProcessId();
		else
		{
			auto pids = blackbone::Process::EnumByName(L"osu!.exe");
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
		// auto path = LR"(C:\WINDOWS\system32\notepad.exe)";
		auto path = LR"(I:\Software\osu\osu!.exe)";
		unsigned long pid;
		const auto instanceHandle = EasyHookInjectStartup(
			path, L"", 0, folderpath.c_str(), &pid);
		
		InstallHook(instanceHandle, ProtoHookIDs::MessageBoxHookID);
		InstallHook(instanceHandle, ProtoHookIDs::RegisterRawInputHookID);
		InstallHook(instanceHandle, ProtoHookIDs::GetRawInputDataHookID);
		WakeUpProcess(instanceHandle);
	}

	// Don't want to end the pipe immediately
	MessageBoxW(NULL, L"Close to exit program", L"", MB_OK);
}
