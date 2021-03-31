#pragma once

// This file needs to be used in both ProtoInputHooks and ProtoInputLoader for pipe communication

namespace ProtoPipe
{

enum class PipeMessageType
{
	SetupHook,
	WakeUpProcess,
	SetupMessageFilter,
	SetupMessageBlock,
	UpdateMainWindowHandle,
	StartFocusMessageLoop,
	SetupState,
	SetupMessagesToSend,
	SetDrawFakeCursor,
	SetExternalFreezeFakeInput
};

struct PipeMessageHeader
{
	PipeMessageType messageType;
	unsigned int messageSize;
};

struct PipeMessageSetupHook
{
	ProtoHookIDs hookID;
	bool install;
};

struct PipeMessageSetupMessageFilter
{
	ProtoMessageFilterIDs filterID;
	bool enable;
};

struct PipeMessageSetupMessageBlock
{
	unsigned int message;
	bool block;
};

struct PipeMessageWakeUpProcess
{
};

struct PipeMesasgeUpdateMainWindowHandle
{
	uint64_t hwnd = 0;
};

struct PipeMessageStartFocusMessageLoop
{
	int milliseconds;
	bool wm_activate;
	bool wm_activateapp;
	bool wm_ncactivate;
	bool wm_setfocus;
	bool wm_mouseactivate;
};

struct PipeMessageSetupState
{
	int instanceNumber;
};

struct PipeMessageSetupMessagesToSend
{
	bool sendMouseWheelMessages;
	bool sendMouseButtonMessages;
	bool sendMouseMoveMessages;
	bool sendKeyboardPressMessages;
};

struct PipeMessageSetDrawFakeCursor
{
	bool enable;
};

struct PipeMessageSetExternalFreezeFakeInput
{
	bool freezeEnabled;
};

}
