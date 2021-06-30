#pragma once
#include <cstdint>

// 0 means invalid handle
using ProtoInstanceHandle = unsigned int;

enum ProtoHookIDs : unsigned int
{
	RegisterRawInputHookID = 0,
	GetRawInputDataHookID,
	MessageFilterHookID,
	GetCursorPosHookID,
	SetCursorPosHookID,
	GetKeyStateHookID,
	GetAsyncKeyStateHookID,
	GetKeyboardStateHookID,
	CursorVisibilityStateHookID,
	ClipCursorHookID,
	FocusHooksHookID,
	RenameHandlesHookID,
	XinputHookID
};

enum ProtoMessageFilterIDs : unsigned int
{
	RawInputFilterID = 0,
	MouseMoveFilterID,
	MouseActivateFilterID,
	WindowActivateFilterID,
	WindowActivateAppFilterID,
	MouseWheelFilterID,
	MouseButtonFilterID,
	KeyboardButtonFilterID
};

extern "C" __declspec(dllexport) ProtoInstanceHandle RemoteLoadLibraryInjectRuntime(unsigned long pid, const wchar_t* dllFolderPath);

extern "C" __declspec(dllexport) ProtoInstanceHandle EasyHookInjectRuntime(unsigned long pid, const wchar_t* dllFolderPath);
extern "C" __declspec(dllexport) ProtoInstanceHandle EasyHookStealthInjectRuntime(unsigned long pid, const wchar_t* dllFolderPath);

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

extern "C" __declspec(dllexport) void SetupState(ProtoInstanceHandle instanceHandle, int instanceIndex);

extern "C" __declspec(dllexport) void SetupMessagesToSend(ProtoInstanceHandle instanceHandle,
														  bool sendMouseWheelMessages = true, bool sendMouseButtonMessages = true, bool sendMouseMoveMessages = true, bool sendKeyboardPressMessages = true);

extern "C" __declspec(dllexport) void StartFocusMessageLoop(ProtoInstanceHandle instanceHandle, int milliseconds = 5,
															bool wm_activate = true, bool wm_activateapp = true, bool wm_ncactivate = true, bool wm_setfocus = true, bool wm_mouseactivate = true);



extern "C" __declspec(dllexport) void SetDrawFakeCursor(ProtoInstanceHandle instanceHandle, bool enable);

extern "C" __declspec(dllexport) void SetExternalFreezeFakeInput(ProtoInstanceHandle instanceHandle, bool enableFreeze);

extern "C" __declspec(dllexport) void AddSelectedMouseHandle(ProtoInstanceHandle instanceHandle, unsigned int mouseHandle);
extern "C" __declspec(dllexport) void AddSelectedKeyboardHandle(ProtoInstanceHandle instanceHandle, unsigned int keyboardHandle);

extern "C" __declspec(dllexport) void SetControllerIndex(ProtoInstanceHandle instanceHandle, unsigned int controllerIndex);

// This MUST be called before calling InstallHook on the Xinput hook
extern "C" __declspec(dllexport) void SetUseDinputRedirection(ProtoInstanceHandle instanceHandle, bool useRedirection);

extern "C" __declspec(dllexport) void SetUseOpenXinput(ProtoInstanceHandle instanceHandle, bool useOpenXinput);

// Both of these functions require RenameHandlesHookHookID hook
extern "C" __declspec(dllexport) void AddHandleToRename(ProtoInstanceHandle instanceHandle, const wchar_t* name);