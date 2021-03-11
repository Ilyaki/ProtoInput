#pragma once

// 0 means invalid handle
using ProtoInstanceHandle  = unsigned int;

enum ProtoHookIDs : unsigned int
{
	MessageBoxHookID = 0
};

extern "C" __declspec(dllexport) ProtoInstanceHandle InjectRuntime(unsigned long pid, const wchar_t* dllFolderPath);

extern "C" __declspec(dllexport) void InstallHook(ProtoInstanceHandle instanceHandle, ProtoHookIDs hookID);
extern "C" __declspec(dllexport) void UninstallHook(ProtoInstanceHandle instanceHandle, ProtoHookIDs hookID);