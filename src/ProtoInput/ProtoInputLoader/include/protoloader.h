#pragma once

// 0 means invalid handle
using ProtoInstanceHandle  = unsigned int;

enum ProtoHookIDs : unsigned int
{
	MessageBoxHookID = 0,
	RegisterRawInputHookID,
	GetRawInputDataHookID,
	MessageFilterHookID
};

enum ProtoMessageFilterIDs : unsigned int
{
	TestFilterID = 0,
};

//TODO: add some documentation to the header file

extern "C" __declspec(dllexport) ProtoInstanceHandle BlackBoneInjectRuntime(unsigned long pid, const wchar_t* dllFolderPath);

extern "C" __declspec(dllexport) ProtoInstanceHandle EasyHookInjectStartup(
	const wchar_t* exePath, 
	const wchar_t* commandLine,
	unsigned long processCreationFlags,
	const wchar_t* dllFolderPath,
	unsigned long* outPid);

extern "C" __declspec(dllexport) void InstallHook(ProtoInstanceHandle instanceHandle, ProtoHookIDs hookID);
extern "C" __declspec(dllexport) void UninstallHook(ProtoInstanceHandle instanceHandle, ProtoHookIDs hookID);

extern "C" __declspec(dllexport) void EnableMessageFilter(ProtoInstanceHandle instanceHandle, ProtoMessageFilterIDs filterID);
extern "C" __declspec(dllexport) void DisableMessageFilter(ProtoInstanceHandle instanceHandle, ProtoMessageFilterIDs filterID);

extern "C" __declspec(dllexport) void EnableMessageBlock(ProtoInstanceHandle instanceHandle, unsigned int messageID);
extern "C" __declspec(dllexport) void DisableMessageBlock(ProtoInstanceHandle instanceHandle, unsigned int messageID);

extern "C" __declspec(dllexport) void WakeUpProcess(ProtoInstanceHandle instanceHandle);