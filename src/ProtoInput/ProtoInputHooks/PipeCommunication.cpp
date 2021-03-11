#include <string>
#include <Windows.h>
#include "PipeCommunication.h"
#include <iostream>
#include "protoloader.h"
#include "HookManager.h"

namespace Proto
{

struct NamedPipeMessage
{
	ProtoHookIDs hookID;
	bool install;
};

std::wstring GetPipeName()
{
	auto pid = GetCurrentProcessId();
	return std::wstring{ LR"(\\.\pipe\)" } + std::to_wstring(pid);
}

DWORD WINAPI PipeThread(LPVOID lpParameter)
{
	// Using printf because cout keeps interleaving with the other threads...
	
	const auto pipeName = GetPipeName();
	wprintf(L"Started pipe thread, using pipe %s\n", pipeName.c_str());
	
	HANDLE pipe = CreateFile(
		pipeName.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (pipe == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Failed to open pipe\n");
	}
	else
	{
		NamedPipeMessage buff;
		DWORD numBytesRead = 0;
		while (true)
		{
			BOOL success = ReadFile(
				pipe,
				&buff,
				sizeof(NamedPipeMessage),
				&numBytesRead,
				NULL
			);

			if (success == 0)
			{
				fprintf(stderr, "Failed to read from pipe, last error = 0x%X\n", GetLastError());
				break;
			}

			printf("Received pipe message, hook ID %d, install = %d\n", buff.hookID, buff.install);

			if (buff.install)
				HookManager::InstallHook(buff.hookID);
			else
				HookManager::UninstallHook(buff.hookID);	
		}
	}

	CloseHandle(pipe);

	return 0;
}

void StartPipeCommunication()
{
	HANDLE hThread = CreateThread(nullptr, 0,
								  (LPTHREAD_START_ROUTINE)PipeThread, GetModuleHandle(0), 0, 0);
	if (hThread != nullptr)
		CloseHandle(hThread);
}

}
