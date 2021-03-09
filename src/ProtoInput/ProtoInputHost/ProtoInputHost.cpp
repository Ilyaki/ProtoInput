// ProtoInputHost.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <BlackBone/Process/Process.h>
#include <BlackBone/Patterns/PatternSearch.h>
#include <BlackBone/Process/RPC/RemoteFunction.hpp>
#include <BlackBone/Syscalls/Syscall.h>
#include <BlackBone/LocalHook/LocalHook.hpp>



int main()
{
	wchar_t pathchars[MAX_PATH];
	GetModuleFileNameW(NULL, pathchars, MAX_PATH);
	std::wstring folderpath = pathchars;
	size_t pos = folderpath.find_last_of(L"\\");
	if (pos != std::string::npos)
		folderpath = folderpath.substr(0, pos + 1);
	
	std::wcout << L"Folder name = '" << folderpath << "'" <<  std::endl;

	//TODO 64/32
	auto dllpath = folderpath + L"ProtoInputHooks.dll";


	constexpr bool hookSelf = true;

	if (hookSelf)
	{
		auto dllhandle = LoadLibraryW(dllpath.c_str());
		std::cout << "Loaded dll, handle = " << dllhandle << std::endl;
		MessageBoxW(NULL, L"Close to exit program", L"", MB_OK);
	}
	else
	{
		auto pids = blackbone::Process::EnumByName(L"notepad.exe");
		for (const auto& pid : pids)
		{
			std::cout << "Selected pid " << pid << std::endl;

			blackbone::Process proc;
			proc.Attach(pid);

			if (auto pLoadLibrary =
				blackbone::MakeRemoteFunction<decltype(&LoadLibraryW)>(proc, L"kernel32.dll", "LoadLibraryW"))
			{
				//auto callRes = pLoadLibrary(LR"(I:\Projects\ProtoInput\src\ProtoInput\x86\Debug\ProtoInputHooks.dll)");
				//auto callRes = pLoadLibrary(LR"(I:\Projects\ProtoInput\src\ProtoInput\x64\Debug\ProtoInputHooks.dll)");
				auto callRes = pLoadLibrary(dllpath.c_str());
				if (!callRes.success())
					std::cerr << "Remote call failed" << std::endl;
				else if (callRes.result() == NULL)
					std::cerr << "Failed to load DLL" << ", error = 0x" << std::hex << GetLastError() << std::dec << std::endl;
				else
					std::cout << "LoadLibrary remote call success\n" << std::endl;
			}
		}
	}
}
