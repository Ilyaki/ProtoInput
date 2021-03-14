#include "RawInput.h"
#include "Gui.h"
#include <cassert>
#include <windows.h>
#include <iostream>
#include <vector>
#include "HookManager.h"

namespace Proto
{

RawInputState RawInput::rawInputState{};
std::bitset<9> RawInput::usages;
std::vector<HWND> RawInput::forwardingWindows{};
bool RawInput::forwardRawInput = true;


RAWINPUT RawInput::inputBuffer[RawInputBufferSize];
std::vector<RAWINPUT> RawInput::rawinputs{};

const std::vector<USAGE> RawInput::usageTypesOfInterest
{
		HID_USAGE_GENERIC_POINTER,
		HID_USAGE_GENERIC_MOUSE,
		HID_USAGE_GENERIC_JOYSTICK,
		HID_USAGE_GENERIC_GAMEPAD,
		HID_USAGE_GENERIC_KEYBOARD,
		HID_USAGE_GENERIC_KEYPAD,
		HID_USAGE_GENERIC_MULTI_AXIS_CONTROLLER
};

HWND RawInput::rawInputHwnd = nullptr;

void RawInput::ProcessRawInput(HRAWINPUT rawInputHandle, bool inForeground, const MSG& msg)
{
	// printf("!");
	
	//TODO: error checking
	RAWINPUT rawinput;
	UINT cbSize;
	GetRawInputData(rawInputHandle, RID_INPUT, nullptr, &cbSize, sizeof(RAWINPUTHEADER));
	GetRawInputData(rawInputHandle, RID_INPUT, &rawinput, &cbSize, sizeof(RAWINPUTHEADER));
	
	//TODO: handle forwarding HID input
	
	if (forwardRawInput)
	{
		if ((rawinput.header.dwType == RIM_TYPEMOUSE && usages[HID_USAGE_GENERIC_MOUSE] && rawinput.header.hDevice == rawInputState.currentMouseHandle) ||
			(rawinput.header.dwType == RIM_TYPEKEYBOARD && usages[HID_USAGE_GENERIC_KEYBOARD] && rawinput.header.hDevice == rawInputState.currentKeyboardHandle))
		{
			for (const auto& hwnd : forwardingWindows)
			{
				//PostMessageW(hwnd, msg.message, msg.wParam, msg.lParam);
				//SendMessageW(hwnd, msg.message, msg.wParam, msg.lParam); (Blocks)

				static size_t inputBufferCounter = 0;
				
				//rawinputs.push_back(rawinput);
				inputBufferCounter = (inputBufferCounter + 1) % RawInputBufferSize;
				inputBuffer[inputBufferCounter] = rawinput;

				LPARAM x = (inputBufferCounter) | 0xAB000000;
				// printf("Sending lparam 0x%X\n", x);
				PostMessageW(hwnd, WM_INPUT, RIM_INPUT, x);
			}
		}
	}

}

LRESULT WINAPI RawInputWindowWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	// case WM_SYSCOMMAND:
	// 	if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
	// 		return 0;
	// 	break;
	case WM_DESTROY:
	{
		printf("Raw input window posting quit message\n");
		PostQuitMessage(0);
		return 0;
	}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

DWORD WINAPI RawInputWindowThread(LPVOID lpParameter)
{
	printf("Starting Raw Input window thread\n");

	AddThreadToACL(GetCurrentThreadId());
	
	const auto hinstance = GetModuleHandle(nullptr);
	
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = RawInputWindowWndProc;
	wc.hInstance = hinstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	const wchar_t* className = L"PROTORAWINPUT";
	wc.lpszClassName = className;
	wc.style = CS_OWNDC;

	if (RegisterClass(&wc))
	{
		RawInput::rawInputHwnd = CreateWindowW(
			wc.lpszClassName,
			L"Proto Input: Raw Input window",
			0,
			0, 0, 300, 300,
			nullptr, nullptr,
			hinstance,
			nullptr);

		UpdateWindow(RawInput::rawInputHwnd);

		//TODO: cleanup window
		// DestroyWindow(hwnd);
		// UnregisterClassW(className, wc.hInstance);
	}

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, RawInput::rawInputHwnd, 0U, 0U, PM_REMOVE))
		{
			if (msg.message == WM_INPUT)
			{				
				RawInput::ProcessRawInput((HRAWINPUT)msg.lParam, GET_RAWINPUT_CODE_WPARAM(msg.wParam) == RIM_INPUT, msg);
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}
	}

	printf("End of raw input window thread\n");

	return 0;
}

void RawInput::RefreshDevices()
{
	//TODO: add a refresh button in the UI
	
	unsigned int numDevices = 0;
	GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));
	auto deviceArray = std::make_unique<RAWINPUTDEVICELIST[]>(numDevices);
	GetRawInputDeviceList(&deviceArray[0], &numDevices, sizeof(RAWINPUTDEVICELIST));

	rawInputState.keyboardHandles.clear();
	rawInputState.mouseHandles.clear();
	
	std::cout << "Raw input devices:\n";
	for (unsigned int i = 0; i < numDevices; ++i)
	{
		auto* device = &deviceArray[i];
		std::cout << (device->dwType == RIM_TYPEHID ? "HID" : device->dwType == RIM_TYPEKEYBOARD ? "Keyboard" : "Mouse")
			<< ": " << device->hDevice << std::endl;
		
		if (device->dwType == RIM_TYPEKEYBOARD)
		{
			rawInputState.keyboardHandles.push_back(device->hDevice);
		}
		else if (device->dwType == RIM_TYPEMOUSE)
		{
			rawInputState.mouseHandles.push_back(device->hDevice);
		}
	}

	// Device handle 0
	rawInputState.keyboardHandles.push_back(0);
	rawInputState.mouseHandles.push_back(0);
}

void RawInput::AddWindowToForward(HWND hwnd)
{
	if (auto find = std::find(forwardingWindows.begin(), forwardingWindows.end(), hwnd) == forwardingWindows.end())
	{
		printf("Adding hwnd 0x%X to forwarding list\n", hwnd);
		forwardingWindows.push_back(hwnd);
	}
}

void RawInput::SetUsageBitField(std::bitset<9> _usages)
{
	usages = _usages;
}

std::bitset<9> RawInput::GetUsageBitField()
{
	return usages;
}

void RawInput::InitialiseRawInput()
{
	RefreshDevices();

	HANDLE hThread = CreateThread(nullptr, 0,
								  (LPTHREAD_START_ROUTINE)RawInputWindowThread, GetModuleHandle(nullptr), 0, 0);
	if (hThread != nullptr)
		CloseHandle(hThread);
}

void RawInput::UnregisterGameFromRawInput()
{
	printf("Unregistering game from raw input\n");

	std::vector<RAWINPUTDEVICE> devices{};

	for (const auto& usage : usageTypesOfInterest)
	{
		RAWINPUTDEVICE dev;
		dev.usUsagePage = HID_USAGE_PAGE_GENERIC;
		dev.usUsage = usage;
		dev.dwFlags = RIDEV_REMOVE;
		dev.hwndTarget = NULL;
		devices.push_back(dev);

		auto res = RegisterRawInputDevices(&dev, 1, sizeof(RAWINPUTDEVICE));
		printf("Deregister usage 0x%X: Result 0x%X\n", usage, res);
	}
}

void RawInput::RegisterProtoForRawInput()
{
	int tryCount = 0;
	
	do
	{
		if (rawInputHwnd == nullptr)
		{
			printf("Trying to register for raw input, but window hasn't opened yet. Waiting 2s\n");
			Sleep(2000);
		}
	}
	while (tryCount++ < 5);

	if (rawInputHwnd == nullptr)
	{
		fprintf(stderr, "Raw input window hasn't opened. Giving up registering raw input\n");
		return;
	}

	
	std::vector<RAWINPUTDEVICE> devices{};
	
	for (const auto& usage : usageTypesOfInterest)
	{
		RAWINPUTDEVICE dev;

		dev.usUsagePage = HID_USAGE_PAGE_GENERIC;
		dev.usUsage = usage;
		dev.dwFlags = RIDEV_INPUTSINK;
		dev.hwndTarget = rawInputHwnd;		
		
		devices.push_back(dev);
	}

	if (RegisterRawInputDevices(&devices[0], devices.size(), sizeof(RAWINPUTDEVICE)) == FALSE)
	{
		fprintf(stderr, "Failed to register raw input, last error = 0x%X\n", GetLastError());
	}
	else
		printf("Successfully register raw input\n");
}

}
