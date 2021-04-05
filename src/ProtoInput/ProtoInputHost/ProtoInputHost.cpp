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
#include "protoinpututil.h"
#include <filesystem>

bool CheckBuildTimings(const std::wstring& folderpath)
{
	const auto loaderPath32 = std::filesystem::path{ folderpath }.append("ProtoInputLoader32.dll");
	const auto loaderPath64 = std::filesystem::path{ folderpath }.append("ProtoInputLoader64.dll");

	const auto hookPath32 = std::filesystem::path{ folderpath }.append("ProtoInputHooks32.dll");
	const auto hookPath64 = std::filesystem::path{ folderpath }.append("ProtoInputHooks64.dll");
	
	auto secLoader = abs(std::chrono::duration_cast<std::chrono::seconds>(last_write_time(loaderPath32) - last_write_time(loaderPath64))).count();
	auto secHooks = abs(std::chrono::duration_cast<std::chrono::seconds>(last_write_time(hookPath32) - last_write_time(hookPath64))).count();
	
	constexpr int maximumDurationSec = 40;

	if (secLoader > maximumDurationSec || secHooks > maximumDurationSec)
	{
		return IDCANCEL == MessageBoxW(NULL, L"Hooks32/64 or Loader32/64 are built at very different times\nMake they have both been compiled", L"Warning", MB_OKCANCEL);
	}

	return false;
}

void testgame()
{
	wchar_t pathchars[MAX_PATH];
	GetModuleFileNameW(NULL, pathchars, MAX_PATH);
	std::wstring folderpath = pathchars;
	size_t pos = folderpath.find_last_of(L"\\");
	if (pos != std::string::npos)
		folderpath = folderpath.substr(0, pos + 1);
	
	auto path = LR"(F:\Steam\steamapps\common\PAYDAY 2\payday2_win32_release.exe)";
	// auto path = LR"(C:\WINDOWS\system32\notepad.exe)";
	unsigned long pid;

	ProtoInstanceHandle instanceHandle = EasyHookInjectStartup(
		path, L"", 0, folderpath.c_str(), &pid);

	SetupState(instanceHandle, 1);

	InstallHook(instanceHandle, RegisterRawInputHookID);
	InstallHook(instanceHandle, GetRawInputDataHookID);
	InstallHook(instanceHandle, MessageFilterHookID);
	InstallHook(instanceHandle, GetCursorPosHookID);
	InstallHook(instanceHandle, SetCursorPosHookID);
	InstallHook(instanceHandle, GetKeyStateHookID);
	InstallHook(instanceHandle, GetAsyncKeyStateHookID);
	InstallHook(instanceHandle, GetKeyboardStateHookID);
	InstallHook(instanceHandle, CursorVisibilityStateHookID);
	InstallHook(instanceHandle, ClipCursorHookID);
	InstallHook(instanceHandle, FocusHooksHookID);

	EnableMessageFilter(instanceHandle, RawInputFilterID);
	EnableMessageFilter(instanceHandle, MouseMoveFilterID);
	EnableMessageFilter(instanceHandle, MouseActivateFilterID);
	EnableMessageFilter(instanceHandle, WindowActivateFilterID);
	EnableMessageFilter(instanceHandle, WindowActivateAppFilterID);
	EnableMessageFilter(instanceHandle, MouseWheelFilterID);
	EnableMessageFilter(instanceHandle, MouseButtonFilterID);

	SetupMessagesToSend(instanceHandle);

	StartFocusMessageLoop(instanceHandle);
	
	AddSelectedMouseHandle(instanceHandle, 65597);
	AddSelectedKeyboardHandle(instanceHandle, 65603);
	AddSelectedKeyboardHandle(instanceHandle, 65605);
	AddSelectedKeyboardHandle(instanceHandle, 65607);

	WakeUpProcess(instanceHandle);

	// LockInput(true);
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
	constexpr bool hookSelf = false;
	constexpr bool doTestGame = false;

	if (doTestGame)
		testgame();
	else if (runtime)
	{
		if (hookSelf)
			const auto instanceHandle = RemoteLoadLibraryInjectRuntime(GetCurrentProcessId(), folderpath.c_str());
		else 
		{
			auto pids = blackbone::Process::EnumByName(L"notepad.exe");
			// auto pids = blackbone::Process::EnumByName(L"hl2.exe");
			for (const auto& pid : pids)
			{
				std::cout << "Selected pid " << pid << std::endl;

				// const auto instanceHandle = BlackBoneInjectRuntime(pid, folderpath.c_str());
				// const auto instanceHandle = NtLoadInjectRuntime(pid, folderpath.c_str());
				// const auto instanceHandle = EasyHookInjectRuntime(pid, folderpath.c_str());

				// const auto instanceHandle = BlackBoneInjectRuntime(pid, folderpath.c_str());
				// InstallHook(instanceHandle, ProtoHookIDs::MessageBoxHookID);
				// InstallHook(instanceHandle, ProtoHookIDs::RegisterRawInputHookID);
				// InstallHook(instanceHandle, ProtoHookIDs::GetRawInputDataHookID);
				

			}
		}
	}
	else
	{
		// auto path = LR"(C:\WINDOWS\system32\notepad.exe)";
		// auto path = LR"(C:\Program Files\Notepad++\notepad++.exe)";
		// auto path = LR"(F:\Steam\steamapps\common\PAYDAY 2\payday2_win32_release.exe)";
		auto path = LR"(I:\Software\osu\osu!.exe)";
		unsigned long pid;

		ProtoInstanceHandle instanceHandle = EasyHookInjectStartup(
				path, L"", 0, folderpath.c_str(), &pid);

		SetupState(instanceHandle, 1);

		AddHandleToRename(instanceHandle, L"20f7b388-7444-42cc-9388-c23275781ff8");
		AddNamedPipeToRename(instanceHandle, L"osu!");
		
		InstallHook(instanceHandle, ProtoHookIDs::RenameHandlesHookHookID);
		InstallHook(instanceHandle, ProtoHookIDs::RegisterRawInputHookID);
		
		// AddSelectedMouseHandle(instanceHandle, 65598);
		// AddSelectedKeyboardHandle(instanceHandle, 65600);
		// AddSelectedKeyboardHandle(instanceHandle, 65602);
		// AddSelectedKeyboardHandle(instanceHandle, 65604);
		
		WakeUpProcess(instanceHandle);
	}

	// Don't want to end the pipe immediately
	MessageBoxW(NULL, L"Close to exit ProtoInputHost", L"", MB_OK);
}
