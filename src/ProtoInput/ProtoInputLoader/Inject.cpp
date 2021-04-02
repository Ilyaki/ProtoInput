#include "include/protoloader.h"

#include <iostream>
#include <Windows.h>
#include <BlackBone/Process/Process.h>
#include <BlackBone/Patterns/PatternSearch.h>
#include <BlackBone/Process/RPC/RemoteFunction.hpp>
#include <BlackBone/Syscalls/Syscall.h>
#include <BlackBone/LocalHook/LocalHook.hpp>
#include "TrackedInstances.h"

#include "pipeinclude.h"

#include "easyhook.h"
#include <filesystem>

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

std::wstring GetPipeName(unsigned long pid)
{
	return std::wstring{ LR"(\\.\pipe\)" } +std::to_wstring(pid);
}

void CreateProtoNamedPipe(Proto::ProtoInstance& instance)
{
	auto pipeName = GetPipeName(instance.pid);
	HANDLE handle = CreateNamedPipeW(pipeName.c_str(),
									 PIPE_ACCESS_OUTBOUND,
									 PIPE_TYPE_BYTE,
									 PIPE_UNLIMITED_INSTANCES,
									 32,
									 32,
									 0,
									 NULL);

	if (handle == INVALID_HANDLE_VALUE)
		std::cerr << "Couldn't open named pipe" << std::endl;
	else
	{
		instance.pipeHandle = handle;
	}
}

ProtoInstanceHandle CreateInstanceHandle(unsigned long pid)
{
	auto handle = Proto::instanceCounter++;
	Proto::ProtoInstance instance{};
	instance.pid = pid;
	CreateProtoNamedPipe(instance);
	Proto::instances[handle] = instance;
	return handle;
}

ProtoInstanceHandle EasyHookInjectImpl(unsigned long pid, const wchar_t* dllFolderPath, bool stealth)
{
	const bool is64 = Isx64(pid);
	std::cout << "Target is " << (is64 ? "64-bit" : "32-bit") << std::endl;
	
	auto processPath = std::wstring(dllFolderPath) + (is64 ? L"ProtoInputInjector64.exe" : L"ProtoInputInjector32.exe");

	STARTUPINFOW startupInfo = { 0 };
	startupInfo.cb = sizeof(startupInfo);
	PROCESS_INFORMATION processInfo = { 0 };

	std::wstring commandLineString = std::to_wstring(pid) + (stealth ? L" 1" : L" 0");
	
	BOOL success = CreateProcessW(
		processPath.c_str(), 
		const_cast<LPWSTR>(commandLineString.c_str()),
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&startupInfo, 
		&processInfo);

	if (success)
	{
		ProtoInstanceHandle ret = 0;

		
		WaitForSingleObject(processInfo.hProcess, INFINITE);

		DWORD exitCode;
		GetExitCodeProcess(processInfo.hProcess, &exitCode);

		switch(exitCode)
		{
		case STATUS_INVALID_PARAMETER_4:			
		case STATUS_INVALID_PARAMETER_5:
			{
				printf("Error: The given library could not be found\n");
				break;
			}
		case STATUS_WOW_ASSERTION:
			{
			printf("Error: Tried to inject 32-bit hooks into 64-bit or vice versa\n");
			break;
			}
		case STATUS_NOT_FOUND:
			{
			printf("Could not find target process %d\n", pid);
			break;
			}
		case STATUS_ACCESS_DENIED:
			{
			printf("Target process could not be access properly or remote thread creation failed\n");
			break;
			}
		case 0:
			{
			printf("Successful EasyHook return value\n");
			break;
			}
		default:
			{
			printf("EasyHook NTSTATUS error: 0x%X\n", exitCode);
			break;
			}
		}

		if (exitCode == 0)
		{
			ret = CreateInstanceHandle(pid);
		}
		
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);

		return ret;
	}
	else
	{
		printf("Failed to create injector sub process\n");
		return 0;
	}
}

extern "C" __declspec(dllexport) ProtoInstanceHandle EasyHookInjectRuntime(unsigned long pid, const wchar_t* dllFolderPath)
{
	return EasyHookInjectImpl(pid, dllFolderPath, false);
}

extern "C" __declspec(dllexport) ProtoInstanceHandle EasyHookStealthInjectRuntime(unsigned long pid, const wchar_t* dllFolderPath)
{
	return EasyHookInjectImpl(pid, dllFolderPath, true);
}

extern "C" __declspec(dllexport) ProtoInstanceHandle BlackBoneInjectRuntime(unsigned long pid, const wchar_t* dllFolderPath)
{
	const bool is64 = Isx64(pid);
	std::cout << "Target is " << (is64 ? "64-bit" : "32-bit") << std::endl;
	
	blackbone::Process proc;
	proc.Attach(pid);
	
	auto dllpath = std::wstring(dllFolderPath);
	dllpath += Isx64(pid) ? L"ProtoInputHooks64.dll" : L"ProtoInputHooks32.dll";
	std::wcout << L"Using dll \"" << dllpath << L"\"" << std::endl;

	// Blackbone LoadLibrary injection method
	if (auto pLoadLibrary =
		blackbone::MakeRemoteFunction<decltype(&LoadLibraryW)>(proc, L"kernel32.dll", "LoadLibraryW"))
	{
		auto callRes = pLoadLibrary(dllpath.c_str());
		if (!callRes.success())
		{
			std::cerr << "Remote call failed" << std::endl;
			return 0;
		}
		else if (callRes.result() == NULL)
		{
			std::cerr << "Failed to load DLL" << ", GetLastError = 0x" << std::hex << GetLastError() << std::dec << std::endl;
			return 0;
		}
		else
		{
			std::cout << "LoadLibrary remote call success\n" << std::endl;
			return CreateInstanceHandle(pid);
		}
	}

	return 0;
}

extern "C" __declspec(dllexport) ProtoInstanceHandle EasyHookInjectStartup(
	const wchar_t* exePath,
	const wchar_t* commandLine,
	unsigned long processCreationFlags,
	const wchar_t* dllFolderPath,
	unsigned long* outPid)
{
	const auto dllpath = std::wstring(dllFolderPath);
	const auto dllpath32 = dllpath + L"ProtoInputHooks32.dll";
	const auto dllpath64 = dllpath + L"ProtoInputHooks64.dll";

	unsigned long pid;

	auto res = RhCreateAndInject(const_cast<WCHAR*>(exePath), const_cast<WCHAR*>(commandLine),
								 processCreationFlags, 0,
								 const_cast<WCHAR*>(dllpath32.c_str()), const_cast<WCHAR*>(dllpath64.c_str()),
								 nullptr, 0, &pid);

	if (FAILED(res))
	{
		std::cerr << "Failed CreateAndInject, NTSTATUS = 0x" << std::hex << res << std::dec << std::endl;
		*outPid = 0;
	}
	else
	{
		*outPid = pid;
		return CreateInstanceHandle(pid);
	}
}

template<typename Body>
void ProtoSendPipeMessage(HANDLE pipe, ProtoPipe::PipeMessageType messageType, Body* body)
{
	ProtoPipe::PipeMessageHeader header
	{
		messageType,
		sizeof(Body)
	};

	DWORD numBytesWritten = 0;
	BOOL write = WriteFile(
		pipe,
		&header,
		sizeof(ProtoPipe::PipeMessageHeader),
		&numBytesWritten,
		NULL
	);

	if (write)
	{
		// std::cout << "Successfully sent message header" << std::endl;
	}
	else
		std::cerr << "Failed to send message header" << std::endl;

	write = WriteFile(
		pipe,
		body,
		sizeof(Body),
		&numBytesWritten,
		NULL
	);
	
	if (write)
	{
		// std::cout << "Successfully sent message body" << std::endl;
	}
	else
		std::cerr << "Failed to send message body" << std::endl;
}

void WaitClientConnect(Proto::ProtoInstance& instance)
{
	if (!instance.clientConnected)
	{
		//FIXME: This NEEDs a timeout
		std::cout << "Starting named pipe wait" << std::endl;

		if (ConnectNamedPipe(instance.pipeHandle, NULL))
		{
			std::cout << "Connected named pipe to pid " << instance.pid << std::endl;
			instance.clientConnected = true;
		}
		else
		{
			std::cerr << "Couldn't connect named pipe to pid " << instance.pid << std::endl;
			return;
		}
	}
}

void InstallUninstallHook(ProtoInstanceHandle instanceHandle, ProtoHookIDs hookID, bool install)
{
	if (const auto find = Proto::instances.find(instanceHandle); find != Proto::instances.end())
	{
		auto& instance = find->second;

		WaitClientConnect(instance);

		ProtoPipe::PipeMessageSetupHook message
		{
			hookID,
			install
		};

		ProtoSendPipeMessage(instance.pipeHandle, ProtoPipe::PipeMessageType::SetupHook, &message);
	}
}

extern "C" __declspec(dllexport) void InstallHook(ProtoInstanceHandle instanceHandle, ProtoHookIDs hookID)
{
	InstallUninstallHook(instanceHandle, hookID, true);
}

extern "C" __declspec(dllexport) void UninstallHook(ProtoInstanceHandle instanceHandle, ProtoHookIDs hookID)
{
	InstallUninstallHook(instanceHandle, hookID, false);
}

void EnableDisableMessageFilter(ProtoInstanceHandle instanceHandle, ProtoMessageFilterIDs filterID, bool enable)
{
	if (const auto find = Proto::instances.find(instanceHandle); find != Proto::instances.end())
	{
		auto& instance = find->second;

		WaitClientConnect(instance);

		ProtoPipe::PipeMessageSetupMessageFilter message
		{
			filterID,
			enable
		};

		ProtoSendPipeMessage(instance.pipeHandle, ProtoPipe::PipeMessageType::SetupMessageFilter, &message);
	}
}

extern "C" __declspec(dllexport) void EnableMessageFilter(ProtoInstanceHandle instanceHandle, ProtoMessageFilterIDs filterID)
{
	EnableDisableMessageFilter(instanceHandle, filterID, true);
}

extern "C" __declspec(dllexport) void DisableMessageFilter(ProtoInstanceHandle instanceHandle, ProtoMessageFilterIDs filterID)
{
	EnableDisableMessageFilter(instanceHandle, filterID, false);
}

void EnableDisableMessageBlock(ProtoInstanceHandle instanceHandle, unsigned int messageID, bool block)
{
	if (const auto find = Proto::instances.find(instanceHandle); find != Proto::instances.end())
	{
		auto& instance = find->second;

		WaitClientConnect(instance);

		ProtoPipe::PipeMessageSetupMessageBlock message
		{
			messageID,
			block
		};

		ProtoSendPipeMessage(instance.pipeHandle, ProtoPipe::PipeMessageType::SetupMessageBlock, &message);
	}
}

extern "C" __declspec(dllexport) void EnableMessageBlock(ProtoInstanceHandle instanceHandle, unsigned int messageID)
{
	EnableDisableMessageBlock(instanceHandle, messageID, true);
}

extern "C" __declspec(dllexport) void DisableMessageBlock(ProtoInstanceHandle instanceHandle, unsigned int messageID)
{
	EnableDisableMessageBlock(instanceHandle, messageID, false);
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