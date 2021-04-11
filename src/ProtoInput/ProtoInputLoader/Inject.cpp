#include "include/protoloader.h"

#include <iostream>
#include <Windows.h>
#include <BlackBone/Process/Process.h>
#include <BlackBone/Patterns/PatternSearch.h>
#include "TrackedInstances.h"

#include "pipeinclude.h"

#include "easyhook.h"
#include <filesystem>

#include "Loader.h"

extern "C" NTSTATUS __declspec(dllexport) __stdcall HookCompleteInjection(void* InInfo)
{
	return HookCompleteInjectionImpl(InInfo);
}

// extern "C" void __declspec(dllexport) __cdecl WakeupProcess()
// {
// 	RhWakeUpProcess();
// }

bool Isx64(unsigned long pid)
{
	blackbone::Process proc;
	proc.Attach(pid);

	SYSTEM_INFO systemInfo;
	GetNativeSystemInfo(&systemInfo);
	
	bool is32 = proc.barrier().targetWow64 || 
		(systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL); // x86
	return !is32;
}

extern "C" __declspec(dllexport) void StartFocusMessageLoop(ProtoInstanceHandle instanceHandle, int milliseconds,
	bool wm_activate, bool wm_activateapp, bool wm_ncactivate, bool wm_setfocus, bool wm_mouseactivate)
{
	if (const auto find = Proto::instances.find(instanceHandle); find != Proto::instances.end())
	{
		auto& instance = find->second;

		WaitClientConnect(instance);

		ProtoPipe::PipeMessageStartFocusMessageLoop message
		{
			milliseconds,
			wm_activate,
			wm_activateapp,
			wm_ncactivate,
			wm_setfocus,
			wm_mouseactivate
		};

		ProtoSendPipeMessage(instance.pipeHandle, ProtoPipe::PipeMessageType::StartFocusMessageLoop, &message);
	}
}

void SetDrawFakeCursor(ProtoInstanceHandle instanceHandle, bool enable)
{
	if (const auto find = Proto::instances.find(instanceHandle); find != Proto::instances.end())
	{
		auto& instance = find->second;

		WaitClientConnect(instance);

		ProtoPipe::PipeMessageSetDrawFakeCursor message
		{
			enable
		};

		ProtoSendPipeMessage(instance.pipeHandle, ProtoPipe::PipeMessageType::SetDrawFakeCursor, &message);
	}
}

extern "C" __declspec(dllexport) void SetExternalFreezeFakeInput(ProtoInstanceHandle instanceHandle, bool enableFreeze)
{
	if (const auto find = Proto::instances.find(instanceHandle); find != Proto::instances.end())
	{
		auto& instance = find->second;

		WaitClientConnect(instance);

		ProtoPipe::PipeMessageSetExternalFreezeFakeInput message
		{
			enableFreeze
		};

		ProtoSendPipeMessage(instance.pipeHandle, ProtoPipe::PipeMessageType::SetExternalFreezeFakeInput, &message);
	}
}

void AddSelectedInputHandleImpl(ProtoInstanceHandle instanceHandle, unsigned int handle, bool mouse)
{
	if (const auto find = Proto::instances.find(instanceHandle); find != Proto::instances.end())
	{
		auto& instance = find->second;

		WaitClientConnect(instance);

		ProtoPipe::PipeMesasgeAddSelectedMouseOrKeyboard message
		{
			mouse ? handle : -1,
			mouse ? -1 : handle
		};

		ProtoSendPipeMessage(instance.pipeHandle, ProtoPipe::PipeMessageType::AddSelectedMouseOrKeyboard, &message);
	}
}

extern "C" __declspec(dllexport) void AddSelectedMouseHandle(ProtoInstanceHandle instanceHandle, unsigned int mouseHandle)
{
	AddSelectedInputHandleImpl(instanceHandle, mouseHandle, true);
}

extern "C" __declspec(dllexport) void AddSelectedKeyboardHandle(ProtoInstanceHandle instanceHandle, unsigned int keyboardHandle)
{
	AddSelectedInputHandleImpl(instanceHandle, keyboardHandle, false);
}

extern "C" __declspec(dllexport) void SetupState(ProtoInstanceHandle instanceHandle, int instanceIndex)
{
	if (instanceIndex < 1)
	{
		fprintf(stderr, "Instance index out of range (must start at 1)\n");
		return;
	}
	
	if (const auto find = Proto::instances.find(instanceHandle); find != Proto::instances.end())
	{
		auto& instance = find->second;

		WaitClientConnect(instance);

		ProtoPipe::PipeMessageSetupState message
		{
			instanceIndex
		};

		ProtoSendPipeMessage(instance.pipeHandle, ProtoPipe::PipeMessageType::SetupState, &message);
	}
}

void AddHandleToRenameImpl(ProtoInstanceHandle instanceHandle, const wchar_t* name, bool namedPipe)
{
	if (const auto find = Proto::instances.find(instanceHandle); find != Proto::instances.end())
	{
		auto& instance = find->second;

		WaitClientConnect(instance);

		
		ProtoPipe::PipeMessageAddHandleToRename message{};
		message.isNamedPipe = namedPipe;

		if (wcslen(name) > sizeof(message.buff) / sizeof(wchar_t) - 2)
		{
			fprintf(stderr, "Handle name \"%ws\" is too long and will not be captured\n", name);
			return;
		}

		memset(message.buff, 0, sizeof(message.buff));
		wcscpy_s(message.buff, name);

		ProtoSendPipeMessage(instance.pipeHandle, ProtoPipe::PipeMessageType::AddHandleToRename, &message);
	}
}

extern "C" __declspec(dllexport) void AddHandleToRename(ProtoInstanceHandle instanceHandle, const wchar_t* name)
{
	AddHandleToRenameImpl(instanceHandle, name, false);
}

extern "C" __declspec(dllexport) void AddNamedPipeToRename(ProtoInstanceHandle instanceHandle, const wchar_t* name)
{
	AddHandleToRenameImpl(instanceHandle, name, true);
}

extern "C" __declspec(dllexport) void SetupMessagesToSend(ProtoInstanceHandle instanceHandle, bool sendMouseWheelMessages, bool sendMouseButtonMessages, bool sendMouseMoveMessages, bool sendKeyboardPressMessages)
{
	if (const auto find = Proto::instances.find(instanceHandle); find != Proto::instances.end())
	{
		auto& instance = find->second;

		WaitClientConnect(instance);

		ProtoPipe::PipeMessageSetupMessagesToSend message
		{
			sendMouseWheelMessages,
			sendMouseButtonMessages,
			sendMouseMoveMessages,
			sendKeyboardPressMessages
		};

		ProtoSendPipeMessage(instance.pipeHandle, ProtoPipe::PipeMessageType::SetupMessagesToSend, &message);
	}
}

extern "C" __declspec(dllexport) void WakeUpProcess(ProtoInstanceHandle instanceHandle)
{
	ProtoPipe::PipeMessageWakeUpProcess message
	{
	};

	if (const auto find = Proto::instances.find(instanceHandle); find != Proto::instances.end())
		ProtoSendPipeMessage(find->second.pipeHandle, ProtoPipe::PipeMessageType::WakeUpProcess, &message);
}

extern "C" __declspec(dllexport) void UpdateMainWindowHandle(ProtoInstanceHandle instanceHandle, uint64_t hwnd)
{
	if (const auto find = Proto::instances.find(instanceHandle); find != Proto::instances.end())
	{
		auto& instance = find->second;

		WaitClientConnect(instance);

		ProtoPipe::PipeMesasgeUpdateMainWindowHandle message
		{
			hwnd
		};

		ProtoSendPipeMessage(instance.pipeHandle, ProtoPipe::PipeMessageType::UpdateMainWindowHandle, &message);
	}
}