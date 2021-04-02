#include <string>
#include <Windows.h>
#include "PipeCommunication.h"
#include <iostream>
#include "HookManager.h"

#include "pipeinclude/pipeinclude.h"
#include <imgui.h>

#include <TlHelp32.h>
#include "MessageFilterHook.h"
#include "MessageList.h"
#include "HwndSelector.h"
#include "FocusMessageLoop.h"
#include "StateInfo.h"
#include "RawInput.h"
#include "FakeCursor.h"

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
		pipe = CreateFileW(
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
				break;
			}
		}
	}
	while (pipe == INVALID_HANDLE_VALUE);
	
	if (pipe != INVALID_HANDLE_VALUE)
	{
		printf("Successfully connected pipe \"%ws\"\n", pipeName.c_str());
		
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

			// printf("Received pipe header, message ID %d, message size = %d\n", msgHeader.messageType, msgHeader.messageSize);
	
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
			case ProtoPipe::PipeMessageType::SetupMessageBlock:
				{
					const auto body = reinterpret_cast<ProtoPipe::PipeMessageSetupMessageBlock*>(messageBuffer);
					printf("Setup message block message: message ID %d, block = %d\n", body->message, body->block);
					auto ptr = MessageList::GetBlocked(body->message);
					if (ptr == nullptr)
						fprintf(stderr, "Trying to set message block on message out of range: %d\n", body->message);
					else
					{
						*ptr = body->block;
						printf("Set message block to %s on message %d", body->block ? "enabled" : "disabled", body->message);
					}
					
					break;
				}
			case ProtoPipe::PipeMessageType::WakeUpProcess:
			{
				printf("Wake up process message\n");
					
				// Wake up the threads if they are suspended
				//TODO: we might need to wait (mutex or sleep) for tasks to complete before waking up
				// Although as long as the host installs hooks first it should be fine...
				
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

				HwndSelector::UpdateMainHwnd();
					
				break;
			}
			case ProtoPipe::PipeMessageType::UpdateMainWindowHandle:
			{
				const auto body = reinterpret_cast<ProtoPipe::PipeMesasgeUpdateMainWindowHandle*>(messageBuffer);
					
				if (body->hwnd == 0)
				{
					printf("Received message to research for the main window handle\n");
					HwndSelector::UpdateMainHwnd();
				}
				else
				{
					printf("Received message to set main window handle to hwnd %lld (0x%llX)\n", body->hwnd, body->hwnd);
					// HwndSelector::selectedHwnd = body->hwnd;
					HwndSelector::SetSelectedHwnd((intptr_t)body->hwnd);
				}
					
				break;
			}
			case ProtoPipe::PipeMessageType::StartFocusMessageLoop:
			{
				const auto body = reinterpret_cast<ProtoPipe::PipeMessageStartFocusMessageLoop*>(messageBuffer);

				FocusMessageLoop::sleepMilliseconds = body->milliseconds;
				FocusMessageLoop::messagesToSend.wm_activate = body->wm_activate;
				FocusMessageLoop::messagesToSend.wm_activateapp = body->wm_activateapp;
				FocusMessageLoop::messagesToSend.wm_mouseactivate = body->wm_mouseactivate;
				FocusMessageLoop::messagesToSend.wm_ncactivate = body->wm_ncactivate;
				FocusMessageLoop::messagesToSend.wm_setfocus = body->wm_setfocus;
				FocusMessageLoop::StartMessageLoop();
				
				break;
			}
			case ProtoPipe::PipeMessageType::SetupState:
			{
				const auto body = reinterpret_cast<ProtoPipe::PipeMessageSetupState*>(messageBuffer);

				printf("Received setup state message, instance index = %d\n", body->instanceNumber);
					
				StateInfo::info.instanceIndex = body->instanceNumber;

				break;
			}
			case ProtoPipe::PipeMessageType::SetupMessagesToSend:
			{
				const auto body = reinterpret_cast<ProtoPipe::PipeMessageSetupMessagesToSend*>(messageBuffer);

				printf("Received setup messages to send, send mouse move = %d, send mouse button = %d, send mouse wheel = %d, send keyboard = %d\n", 
					   body->sendMouseMoveMessages ? 1 : 0, body->sendMouseButtonMessages ? 1 : 0, body->sendMouseWheelMessages ? 1 : 0, body->sendKeyboardPressMessages ? 1 : 0);

				RawInput::rawInputState.sendMouseMoveMessages = body->sendMouseMoveMessages;
				RawInput::rawInputState.sendMouseButtonMessages = body->sendMouseButtonMessages;
				RawInput::rawInputState.sendMouseWheelMessages = body->sendMouseWheelMessages;
				RawInput::rawInputState.sendKeyboardPressMessages = body->sendKeyboardPressMessages;
					
				break;
			}
			case ProtoPipe::PipeMessageType::SetDrawFakeCursor:
			{
				const auto body = reinterpret_cast<ProtoPipe::PipeMessageSetDrawFakeCursor*>(messageBuffer);

				printf("Received message to %s fake cursor\n", body->enable ? "enable" : "disable");

				FakeCursor::EnableDisableFakeCursor(body->enable);

				break;
			}
			case ProtoPipe::PipeMessageType::SetExternalFreezeFakeInput:
			{
				const auto body = reinterpret_cast<ProtoPipe::PipeMessageSetExternalFreezeFakeInput*>(messageBuffer);

				printf("Received message to set external freeze fake input to %s\n", body->freezeEnabled ? "enabled" : "disabled");

				RawInput::rawInputState.externalFreezeInput = body->freezeEnabled;
				
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
