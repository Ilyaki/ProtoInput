#include "Loader.h"
#include <easyhook.h>

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

	printf("Dll path 32 = \"%ws\", 64 = \"%ws\"\n", dllpath32.c_str(), dllpath64.c_str());

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