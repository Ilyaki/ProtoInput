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
	// if (auto hmod = LoadLibraryW(LR"(I:\Projects\ProtoInput\src\ProtoInput\x64\Debug\ProtoInputHooks.dll)"); hmod == 0)
	// 	std::cerr << "(local) Failed to load DLL" << ", error = 0x" << std::hex << GetLastError() << std::dec << std::endl;
	// else
	// 	std::cout << "Local load successful" << std::endl;
		
	// auto pids = blackbone::Process::EnumByName(L"osu!.exe");
	auto pids = blackbone::Process::EnumByName(L"notepad.exe");
	// auto pids = blackbone::Process::EnumByName(L"teeworlds.exe");
	for (const auto& pid : pids)
	{
		std::cout << "Selected pid " << pid << std::endl;

		blackbone::Process proc;
		proc.Attach(pid);

		if (auto pLoadLibrary = 
			blackbone::MakeRemoteFunction<decltype(&LoadLibraryW)>(proc, L"kernel32.dll", "LoadLibraryW"))
		{
			//auto callRes = pLoadLibrary(LR"(I:\Projects\ProtoInput\src\ProtoInput\x86\Debug\ProtoInputHooks.dll)");
			auto callRes = pLoadLibrary(LR"(I:\Projects\ProtoInput\src\ProtoInput\x64\Debug\ProtoInputHooks.dll)");
			if (!callRes.success())
				std::cerr << "Remote call failed" << std::endl;
			else if (callRes.result() == NULL)
				std::cerr << "Failed to load DLL" << ", error = 0x" << std::hex << GetLastError() << std::dec << std::endl;
			else
				std::cout << "LoadLibrary remote call success\n" << std::endl;
		}
	}

	// int x;
	// std::cin >> x;
}
