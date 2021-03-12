#include "RawInput.h"
#include "Gui.h"
#include <cassert>
#include <windows.h>
#include <iostream>
#include <vector>

namespace Proto
{

RawInputState RawInput::rawInputState{};

void ProcessRawInput(HRAWINPUT rawInputHandle, bool inForeground)
{
	//printf("!");

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

	const auto hinstance = GetModuleHandle(nullptr);
	
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = RawInputWindowWndProc;
	wc.hInstance = hinstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	const wchar_t* className = L"PROTORAWINPUT";
	wc.lpszClassName = className;
	wc.style = CS_OWNDC;

	HWND rawInputHwnd = nullptr;
	
	if (RegisterClass(&wc))
	{
		rawInputHwnd = CreateWindowW(
			wc.lpszClassName,
			L"Proto Input: Raw Input window",
			0,
			0, 0, 300, 300,
			nullptr, nullptr,
			hinstance,
			nullptr);

		UpdateWindow(rawInputHwnd);

		//TODO: cleanup window
		// DestroyWindow(hwnd);
		// UnregisterClassW(className, wc.hInstance);




		//TODO: need to check if there's an existing raw input window, then add it to a list of windows that we forward input onto.
		// (separate the lists into 3 lists, mouse,kb, HID, as it'll be faster than looking up)
		//TODO: add this thread to the easyhook ACL list (remember to add to ACL before installing any hooks!)
		
		
		{
			RAWINPUTDEVICE Rid[2];

			Rid[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
			Rid[0].usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
			Rid[0].dwFlags = RIDEV_INPUTSINK;
			Rid[0].hwndTarget = rawInputHwnd;

			Rid[1].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
			Rid[1].usUsage = 0x06;              // HID_USAGE_GENERIC_KEYBOARD
			Rid[1].dwFlags = RIDEV_INPUTSINK;
			Rid[1].hwndTarget = rawInputHwnd;

			//TODO: process HID and pass it to the game if it uses it?
			
			if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE)
			{
				std::cerr << "Raw input failed, Error " << GetLastError() << std::endl;
				return 0;
			}

			std::cout << "Registered raw input devices" << std::endl;
		}
	}

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, rawInputHwnd, 0U, 0U, PM_REMOVE))
		{
			if (msg.message == WM_INPUT)
			{
				ProcessRawInput((HRAWINPUT)msg.lParam, GET_RAWINPUT_CODE_WPARAM(msg.wParam) == RIM_INPUT);
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
}

void RawInput::InitialiseRawInput()
{
	RefreshDevices();

	
	HANDLE hThread = CreateThread(nullptr, 0,
								  (LPTHREAD_START_ROUTINE)RawInputWindowThread, GetModuleHandle(nullptr), 0, 0);
	if (hThread != nullptr)
		CloseHandle(hThread);
}

}
