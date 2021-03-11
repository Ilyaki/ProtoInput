#include <string>
#include <Windows.h>
#include "PipeCommunication.h"
#include <iostream>
#include "protoloader.h"
#include "HookManager.h"

#include "pipeinclude/pipeinclude.h"

namespace Proto
{

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

	// Just in case the dll is injected first, before the loader starts the pipe
	HANDLE pipe;
	int connectCount = 0;
	do
	{
		pipe = CreateFile(
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
			if (connectCount++ < 10)
			{
				fprintf(stderr, "Failed to open pipe, trying again in 2s\n");
				Sleep(2000);
			}
			else
			{
				fprintf(stderr, "Failed to open pipe, giving up!\n");
			}
		}
	}
	while (pipe == INVALID_HANDLE_VALUE);
	
	if (pipe != INVALID_HANDLE_VALUE)
	{
		ProtoPipe::PipeMessageHeader msgHeader;
		unsigned char messageBuffer[256];

		DWORD numBytesRead = 0;
		while (true)
		{
			BOOL success = ReadFile(
				pipe,
				&msgHeader,
				sizeof(ProtoPipe::PipeMessageHeader),
				&numBytesRead,
				NULL
			);

			if (success == 0)
			{
				fprintf(stderr, "Failed to read message header from pipe, last error = 0x%X\n", GetLastError());
				break;
			}

			printf("Received pipe header, message ID %d, message size = %d\n", msgHeader.messageType, msgHeader.messageSize);
	
			success = ReadFile(
				pipe,
				messageBuffer,
				msgHeader.messageSize,
				&numBytesRead,
				NULL
			);

			if (success == 0)
			{
				fprintf(stderr, "Failed to read message body from pipe, last error = 0x%X\n", GetLastError());
				break;
			}

			switch(msgHeader.messageType)
			{
			case ProtoPipe::PipeMessageType::SetupHook:
				{
					const auto body = reinterpret_cast<ProtoPipe::PipeMessageSetupHook*>(messageBuffer);
					printf("Setup hook message: hook ID %d, install = %d\n", body->hookID, body->install);
					if (body->install)
						HookManager::InstallHook(body->hookID);
					else
						HookManager::UninstallHook(body->hookID);

					break;
				}				
			default:
				{
					fprintf(stderr, "Unrecongnised message type, exiting pipe\n");
					goto endPipe;
				}
			}
		}
	}
	endPipe:

	printf("End of pipe thread\n");
	
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
