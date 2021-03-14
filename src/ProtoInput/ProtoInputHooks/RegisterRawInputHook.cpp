#include <windows.h>
#include "RegisterRawInputHook.h"
#include <cassert>
#include <hidusage.h>
#include <bitset>
#include "RawInput.h"
#include <imgui.h>

namespace Proto
{

//TODO: add to ui
//FIXME: turn this false or it will spam the log for some games
bool logCallsToRegisterRawInput = true;
RegisterRawInputHook* registerRawInputHookPtr = nullptr;

BOOL WINAPI Hook_RegisterRawInputDevices(PCRAWINPUTDEVICE pRawInputDevices, UINT uiNumDevices, UINT cbSize)
{
	if (logCallsToRegisterRawInput)
		printf("Detected a call to RegisterRawInputDevices");
	
	HWND targetHWND = NULL;
	std::bitset<9> usagesToForward{};

	for (size_t i = 0; i < uiNumDevices; i++)
	{
		//TODO: hwndTarget can be NULL if raw input follows the keyboard focus.
		const auto targetI = pRawInputDevices[i].hwndTarget;
		if (targetI != NULL && targetI != RawInput::rawInputHwnd)
		{
			if (targetHWND != NULL && targetHWND != targetI && logCallsToRegisterRawInput)
				printf("WARNING: The game appears to have multiple hwnds subscribed to raw input. We are only using one\n");

			targetHWND = targetI;

			const auto usage = pRawInputDevices[i].usUsage;

			if (usage < HID_USAGE_GENERIC_POINTER || usage > HID_USAGE_GENERIC_MULTI_AXIS_CONTROLLER)
				fprintf(stderr, "Raw input usUsage out of range\n");
			else
			{
				usagesToForward[usage] = true;
			}
		}
	}

	if (targetHWND == nullptr)
	{
		if (logCallsToRegisterRawInput)
			printf("Couldn't find a hwnd subscribed to raw input (The call was probably to unsubscribe)\n");
	}
	else
	{
		if (logCallsToRegisterRawInput)
			printf("Found raw input hwnd: 0x%X\n", targetHWND);
		
		registerRawInputHookPtr->AddWindowToForward(targetHWND, usagesToForward);
	}

	return TRUE;
}

void RegisterRawInputHook::FindAlreadySubscribedWindows()
{
	UINT numDevices = 0;
	const auto size = sizeof(RAWINPUTDEVICE);
	GetRegisteredRawInputDevices(nullptr, &numDevices, size); // Get the number of devices

	RAWINPUTDEVICE* devices = new RAWINPUTDEVICE[numDevices];
	auto ret = GetRegisteredRawInputDevices(devices, &numDevices, size); // Get the devices

	if (numDevices == 0 || ret <= 0)
	{
		printf("Couldn't a window that's already subscribed to raw input. numDevices = %d, ret = 0x%X, GetLastError = 0x%X\n",
			   numDevices, ret, GetLastError());
	}
	else
	{
		// https://docs.microsoft.com/en-us/windows-hardware/drivers/hid/hid-usages#usage-id
		//	Usage ID	Usage Name 					hidusage.h constant
		//	0x01		Pointer 					HID_USAGE_GENERIC_POINTER
		//	0x02		Mouse 						HID_USAGE_GENERIC_MOUSE
		//	0x04		Joystick 					HID_USAGE_GENERIC_JOYSTICK
		//	0x05		Game Pad 					HID_USAGE_GENERIC_GAMEPAD
		//	0x06		Keyboard 					HID_USAGE_GENERIC_KEYBOARD
		//	0x07		Keypad 						HID_USAGE_GENERIC_KEYPAD
		//	0x08		Multi - axis Controller 	HID_USAGE_GENERIC_MULTI_AXIS_CONTROLLER
		HWND targetHWND = NULL;
		std::bitset<9> usagesToForward{};

		for (size_t i = 0; i < numDevices; i++)
		{
			//TODO: hwndTarget can be NULL if raw input follows the keyboard focus.
			const auto targetI = devices[i].hwndTarget;
			if (targetI != NULL && targetI != RawInput::rawInputHwnd)
			{
				if (targetHWND != NULL && targetHWND != targetI)
					printf("WARNING: The game appears to have multiple hwnds subscribed to raw input. We are only using one\n");

				targetHWND = targetI;

				const auto usage = devices[i].usUsage;

				if (usage < HID_USAGE_GENERIC_POINTER || usage > HID_USAGE_GENERIC_MULTI_AXIS_CONTROLLER)
					fprintf(stderr, "Raw input usUsage out of range\n");
				else
				{
					usagesToForward[usage] = true;
				}
			}
		}

		if (targetHWND == nullptr)
			printf("Couldn't find a hwnd subscribed to raw input\n");
		else
		{
			printf("Found raw input hwnd: 0x%X\n", targetHWND);
			AddWindowToForward(targetHWND, usagesToForward);
		}
	}
}

void RegisterRawInputHook::AddWindowToForward(HWND hwnd, std::bitset<9> usages)
{
	if (updateDeviceTypeOnGameUpdate)
	{
		RawInput::SetUsageBitField(usages);
	}

	RawInput::AddWindowToForward(hwnd);
}

void RegisterRawInputHook::ShowGuiStatus()
{
	ImGui::Checkbox("Forward raw input", &RawInput::forwardRawInput);
}


void WINAPI Hook_ExitProcess(
	UINT uExitCode
)
{
	printf("Blocked ExitProcess\n");
	Sleep(100000);
}

void RegisterRawInputHook::InstallImpl()
{
	if (!installedAtLeastOnce)
	{
		installedAtLeastOnce = true;
		std::bitset<9> usages{};
		usages[HID_USAGE_GENERIC_MOUSE] = true;
		usages[HID_USAGE_GENERIC_KEYBOARD] = true;
		RawInput::SetUsageBitField(usages);
	}

	//FIXME: remove
	InstallNamedHook(L"kernel32", "ExitProcess", Hook_ExitProcess);

	auto [status, _hookInfo] = InstallNamedHook(L"user32", "RegisterRawInputDevices", Hook_RegisterRawInputDevices);
	this->hookInfo = _hookInfo;
		
	FindAlreadySubscribedWindows();

	RawInput::UnregisterGameFromRawInput();
	RawInput::RegisterProtoForRawInput();
}

void RegisterRawInputHook::UninstallImpl()
{
	UninstallHook(&hookInfo);
}

RegisterRawInputHook::RegisterRawInputHook()
{
	registerRawInputHookPtr = this;
}

}
