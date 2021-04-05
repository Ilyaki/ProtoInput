#include "protoloader.h"
#include "Loader.h"
#include <string>
#include "TrackedInstances.h"
#include <iostream>
#include "pipeinclude.h"

std::wstring GetPipeName(unsigned long pid)
{
	return std::wstring{ LR"(\\.\pipe\)" } + std::to_wstring(pid);
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