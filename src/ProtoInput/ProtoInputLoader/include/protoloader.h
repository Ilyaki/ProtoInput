#pragma once
#include <cstdint>

// 0 means invalid handle
using ProtoInstanceHandle  = unsigned int;

enum ProtoHookIDs : unsigned int
{
	MessageBoxHookID = 0,
	RegisterRawInputHookID,
	GetRawInputDataHookID,
	MessageFilterHookID,
	FocusHooksHookID,
	GetCursorPosHookID,
	SetCursorPosHookID,
	GetKeyStateHookID,
	GetAsyncKeyStateHookID,
	GetKeyboardStateHookID
};

enum ProtoMessageFilterIDs : unsigned int
{
	RawInputFilterID,
	MouseMoveFilterID,
	MouseActivateFilterID,
	WindowActivateFilterID,
	WindowActivateAppFilterID,
	MouseWheelFilterID
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

extern "C" __declspec(dllexport) void UpdateMainWindowHandle(ProtoInstanceHandle instanceHandle, uint64_t hwnd = 0);

extern "C" __declspec(dllexport) void StartFocusMessageLoop(ProtoInstanceHandle instanceHandle, int milliseconds = 5,
		bool wm_activate = true, bool wm_activateapp = true, bool wm_ncactivate = true, bool wm_setfocus = true, bool wm_mouseactivate = true);