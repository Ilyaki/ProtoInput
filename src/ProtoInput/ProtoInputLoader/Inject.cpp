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

	bool is32 = proc.barrier().targetWow64 || (sizeof(intptr_t) == 4);
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

extern "C" __declspec(dllexport) ProtoInstanceHandle BlackBoneInjectRuntime(unsigned long pid, const wchar_t* dllFolderPath)
{
	const bool is64 = Isx64(pid);
	std::cout << "Target is " << (is64 ? "64-bit" : "32-bit") << std::endl;
	
	blackbone::Process proc;
	proc.Attach(pid);

	//FIXME: choose the dll whether or not 64
	auto dllpath = std::wstring(dllFolderPath);
	dllpath += L"ProtoInputHooks.dll";
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
			std::cerr << "Failed to load DLL" << ", error = 0x" << std::hex << GetLastError() << std::dec << std::endl;
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

	auto dllpath = std::wstring(dllFolderPath);
	dllpath += L"ProtoInputHooks.dll";

	//FIXME: choose the dll whether or not 64
	const auto dllPath64 = dllpath.c_str();
	const auto dllPath32 = dllpath.c_str();

	unsigned long pid;

	auto res = RhCreateAndInject(const_cast<WCHAR*>(exePath), const_cast<WCHAR*>(commandLine),
								 processCreationFlags, 0,
								 const_cast<WCHAR*>(dllPath32), const_cast<WCHAR*>(dllPath64),
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
		std::cout << "Successfully sent message header" << std::endl;
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
		std::cout << "Successfully sent message body" << std::endl;
	else
		std::cerr << "Failed to send message body" << std::endl;
}

void InstallUninstallHook(ProtoInstanceHandle instanceHandle, ProtoHookIDs hookID, bool install)
{
	if (const auto find = Proto::instances.find(instanceHandle); find != Proto::instances.end())
	{
		auto& instance = find->second;

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

extern "C" __declspec(dllexport) void WakeUpProcess(ProtoInstanceHandle instanceHandle)
{
	ProtoPipe::PipeMessageWakeUpProcess message
	{
	};

	if (const auto find = Proto::instances.find(instanceHandle); find != Proto::instances.end())
		ProtoSendPipeMessage(find->second.pipeHandle, ProtoPipe::PipeMessageType::WakeUpProcess, &message);
}