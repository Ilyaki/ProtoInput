#include <string>
#include <Windows.h>
#include "PipeCommunication.h"
#include <iostream>
#include "HookManager.h"

#include "pipeinclude/pipeinclude.h"
#include <imgui.h>



#include "Gui.h"
#include <TlHelp32.h>
#include "MessageFilterHook.h"


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

	AddThreadToACL(GetCurrentThreadId());

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
			case ProtoPipe::PipeMessageType::SetupMessageFilter:
				{
					const auto body = reinterpret_cast<ProtoPipe::PipeMessageSetupMessageFilter*>(messageBuffer);
					printf("Setup message filter message: filter ID %d, enable = %d\n", body->filterID, body->enable);
					MessageFilterHook::SetFilterEnabled(body->filterID, body->enable);

					break;
				}
			case ProtoPipe::PipeMessageType::WakeUpProcess:
			{
				printf("Wake up process message\n");
					
				// Wake up the threads if they are suspended
				//TODO: we might need to wait (mutex or sleep) for tasks to complete before waking up
				{
					HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
					if (h != INVALID_HANDLE_VALUE) {
						THREADENTRY32 te;
						te.dwSize = sizeof(te);
						if (Thread32First(h, &te)) {
							do {
								if (te.th32OwnerProcessID == GetCurrentProcessId() &&
									(std::find(ACLThreads.begin(), ACLThreads.end(), te.th32OwnerProcessID) == ACLThreads.end()) &&
									te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) +
									sizeof(te.th32OwnerProcessID))
								{
									//TODO: maybe check the module isn't ProtoInputHooks to be safe (dont want to resume one of our threads that should be suspended)

									if (auto hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID); hThread != NULL)
									{
										printf("Waking thread %d\n", te.th32ThreadID);

										ResumeThread(hThread);

										CloseHandle(hThread);
									}
								}
								te.dwSize = sizeof(te);
							}
							while (Thread32Next(h, &te));
						}
						CloseHandle(h);
					}
				}

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
