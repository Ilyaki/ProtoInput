#include "RawInput.h"
#include "Gui.h"
#include <cassert>
#include <windows.h>
#include <iostream>
#include <vector>
#include "HookManager.h"
#include "FakeMouseKeyboard.h"
#include <unordered_map>
#include "HwndSelector.h"
#include "MouseWheelFilter.h"
#include <3rd_party/BeaEngine/headers/Includes/basic_types.h>

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

void RawInput::ProcessMouseInput(const RAWMOUSE& data, HANDLE deviceHandle)
{
	// Update fake mouse position
	if ((data.usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE)
	{
		const bool isVirtualDesktop = (data.usFlags & MOUSE_VIRTUAL_DESKTOP) == MOUSE_VIRTUAL_DESKTOP;

		// const int width = GetSystemMetrics(isVirtualDesktop ? SM_CXVIRTUALSCREEN : SM_CXSCREEN);
		// const int height = GetSystemMetrics(isVirtualDesktop ? SM_CYVIRTUALSCREEN : SM_CYSCREEN);

		static int widthVirtual = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		static int widthNonVirtual = GetSystemMetrics(SM_CXSCREEN);
		static int heightVirtual = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		static int heightNonVirtual = GetSystemMetrics(SM_CYSCREEN);
		
		const int absoluteX = int((data.lLastX / 65535.0f) * (isVirtualDesktop ? widthVirtual : widthNonVirtual));
		const int absoluteY = int((data.lLastY / 65535.0f) * (isVirtualDesktop ? heightVirtual : heightNonVirtual));

		static std::unordered_map<HANDLE, std::pair<int, int>> oldPositions{};
		
		if (const auto find = oldPositions.find(deviceHandle); find != oldPositions.end())
		{
			FakeMouseKeyboard::AddMouseDelta(absoluteX - find->second.first, absoluteY - find->second.second);
		}
		else
		{
			oldPositions.emplace(std::make_pair( deviceHandle, std::pair<int, int>{ absoluteX, absoluteY } ));
		}		
	}
	else if (data.lLastX != 0 || data.lLastY != 0)
	{
		const int relativeX = data.lLastX;
		const int relativeY = data.lLastY;
		FakeMouseKeyboard::AddMouseDelta(relativeX, relativeY);
	}

	// Set vkeys (GetKeyState/etc can be used to get the mouse buttons state)
	if ((data.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) != 0)
		FakeMouseKeyboard::ReceivedKeyPressOrRelease(VK_LBUTTON, true);
	if ((data.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) != 0)
		FakeMouseKeyboard::ReceivedKeyPressOrRelease(VK_LBUTTON, false);

	if ((data.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) != 0)
		FakeMouseKeyboard::ReceivedKeyPressOrRelease(VK_MBUTTON, true);
	if ((data.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) != 0)
		FakeMouseKeyboard::ReceivedKeyPressOrRelease(VK_MBUTTON, false);

	if ((data.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) != 0)
		FakeMouseKeyboard::ReceivedKeyPressOrRelease(VK_RBUTTON, true);
	if ((data.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) != 0)
		FakeMouseKeyboard::ReceivedKeyPressOrRelease(VK_RBUTTON, false);

	if ((data.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) != 0)
		FakeMouseKeyboard::ReceivedKeyPressOrRelease(VK_XBUTTON1, true);
	if ((data.usButtonFlags & RI_MOUSE_BUTTON_4_UP) != 0)
		FakeMouseKeyboard::ReceivedKeyPressOrRelease(VK_XBUTTON1, false);

	if ((data.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) != 0)
		FakeMouseKeyboard::ReceivedKeyPressOrRelease(VK_XBUTTON2, true);
	if ((data.usButtonFlags & RI_MOUSE_BUTTON_5_UP) != 0)
		FakeMouseKeyboard::ReceivedKeyPressOrRelease(VK_XBUTTON2, false);


	// This is used a lot in sending messages
	const unsigned int mouseMkFlags = FakeMouseKeyboard::GetMouseMkFlags();
	const unsigned int mousePointLparam = MAKELPARAM(FakeMouseKeyboard::GetMouseState().x, FakeMouseKeyboard::GetMouseState().y);
	
	
	// Send mouse wheel
	if (rawInputState.sendMouseWheelMessages)
	{
		if((data.usButtonFlags & RI_MOUSE_WHEEL) != 0)
		{
			const unsigned int wparam = data.usButtonData << 4
				| MouseWheelFilter::protoInputSignature
				| mouseMkFlags;

			const unsigned int lparam = mousePointLparam;
			
			PostMessageW((HWND)HwndSelector::GetSelectedHwnd(), WM_MOUSEWHEEL, wparam, lparam);

		}
	}


	// Send mouse button messages
	if (rawInputState.sendMouseButtonMessages)
	{
		//FIXME: need a filter to only let through synthesized versions of these messages
		
		if ((data.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) != 0)
			PostMessageW((HWND)HwndSelector::GetSelectedHwnd(), WM_LBUTTONDOWN, mouseMkFlags, mousePointLparam);
		if ((data.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) != 0)
			PostMessageW((HWND)HwndSelector::GetSelectedHwnd(), WM_LBUTTONUP, mouseMkFlags, mousePointLparam);

		if ((data.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) != 0)
			PostMessageW((HWND)HwndSelector::GetSelectedHwnd(), WM_MBUTTONDOWN, mouseMkFlags, mousePointLparam);
		if ((data.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) != 0)
			PostMessageW((HWND)HwndSelector::GetSelectedHwnd(), WM_MBUTTONUP, mouseMkFlags, mousePointLparam);

		if ((data.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) != 0)
			PostMessageW((HWND)HwndSelector::GetSelectedHwnd(), WM_RBUTTONDOWN, mouseMkFlags, mousePointLparam);
		if ((data.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) != 0)
			PostMessageW((HWND)HwndSelector::GetSelectedHwnd(), WM_RBUTTONUP, mouseMkFlags, mousePointLparam);

		if ((data.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) != 0)
			PostMessageW((HWND)HwndSelector::GetSelectedHwnd(), WM_XBUTTONDOWN, mouseMkFlags | (XBUTTON1 << 4), mousePointLparam);
		if ((data.usButtonFlags & RI_MOUSE_BUTTON_4_UP) != 0)
			PostMessageW((HWND)HwndSelector::GetSelectedHwnd(), WM_XBUTTONUP, mouseMkFlags | (XBUTTON1 << 4), mousePointLparam);

		if ((data.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) != 0)
			PostMessageW((HWND)HwndSelector::GetSelectedHwnd(), WM_XBUTTONDOWN, mouseMkFlags | (XBUTTON2 << 4), mousePointLparam);
		if ((data.usButtonFlags & RI_MOUSE_BUTTON_5_UP) != 0)
			PostMessageW((HWND)HwndSelector::GetSelectedHwnd(), WM_XBUTTONUP, mouseMkFlags | (XBUTTON2 << 4), mousePointLparam);
	}



	// WM_MOUSEMOVE
	if (rawInputState.sendMouseMoveMessages)
	{
		PostMessageW((HWND)HwndSelector::GetSelectedHwnd(), WM_MOUSEMOVE, mouseMkFlags, mousePointLparam);
	}
}

void RawInput::ProcessKeyboardInput(const RAWKEYBOARD& data, HANDLE deviceHandle)
{
	if ((data.Flags & RI_KEY_MAKE) != 0)
		FakeMouseKeyboard::ReceivedKeyPressOrRelease(data.VKey, true);
	else if ((data.Flags & RI_KEY_BREAK) != 0)
		FakeMouseKeyboard::ReceivedKeyPressOrRelease(data.VKey, false);
}

void RawInput::ProcessRawInput(HRAWINPUT rawInputHandle, bool inForeground, const MSG& msg)
{	
	//TODO: error checking
	RAWINPUT rawinput;
	UINT cbSize;
	GetRawInputData(rawInputHandle, RID_INPUT, nullptr, &cbSize, sizeof(RAWINPUTHEADER));
	GetRawInputData(rawInputHandle, RID_INPUT, &rawinput, &cbSize, sizeof(RAWINPUTHEADER));
	rawinput.header.wParam = RIM_INPUT; // Sent in the foreground
	
	//TODO: this should be passed in by pipe as a state
	constexpr int index = 1;

	// Shortcut to open UI (doesn't care about what keyboard is attached)
	if (rawinput.header.dwType == RIM_TYPEKEYBOARD && index >= 1 && index <= 9 && (rawinput.data.keyboard.VKey == 0x30 + index))
	{
		static bool keyDown = false;
		if (rawinput.data.keyboard.Flags == RI_KEY_MAKE && !keyDown)
		{
			keyDown = true;

			// Key just pressed
			if ((GetAsyncKeyState(VK_RCONTROL) & ~1) != 0 && (GetAsyncKeyState(VK_RMENU) & ~1) != 0)
			{
				Proto::ToggleWindow();
			}			
		}

		if (rawinput.data.keyboard.Flags == RI_KEY_BREAK && keyDown)
		{
			keyDown = false;
		}
	}

	// Need to occasionally update the window is case the main window changes (e.g. because of a launcher) or the main window is resized
	if (rawinput.header.dwType == RIM_TYPEMOUSE && (rawinput.data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) != 0)
	{
		//TODO: This may waste CPU? (But need a way to update window otherwise)
		//if (HwndSelector::GetSelectedHwnd() == 0)
			HwndSelector::UpdateMainHwnd(false);

		HwndSelector::UpdateWindowBounds();
	}

	
	//TODO: handle forwarding HID input
	
	if (forwardRawInput)
	{
		const bool allowMouse = (rawinput.header.dwType == RIM_TYPEMOUSE && usages[HID_USAGE_GENERIC_MOUSE] && std::find(rawInputState.selectedMouseHandles.begin(), rawInputState.selectedMouseHandles.end(), rawinput.header.hDevice) != rawInputState.selectedMouseHandles.end());
		const bool allowKeyboard = (rawinput.header.dwType == RIM_TYPEKEYBOARD && usages[HID_USAGE_GENERIC_KEYBOARD] && std::find(rawInputState.selectedKeyboardHandles.begin(), rawInputState.selectedKeyboardHandles.end(), rawinput.header.hDevice) != rawInputState.selectedKeyboardHandles.end());
		
		if (allowMouse || allowKeyboard)
		{
			if (allowMouse)
				ProcessMouseInput(rawinput.data.mouse, rawinput.header.hDevice);
			else if (allowKeyboard)
				ProcessKeyboardInput(rawinput.data.keyboard, rawinput.header.hDevice);
			
			for (const auto& hwnd : forwardingWindows)
			{
				static size_t inputBufferCounter = 0;

				// The game is going to lag behind the data we get by a few times, so store in an array and pass the index as a message parameter
				
				inputBufferCounter = (inputBufferCounter + 1) % RawInputBufferSize;
				inputBuffer[inputBufferCounter] = rawinput;

				const LPARAM x = (inputBufferCounter) | 0xAB000000;
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
		//if (PeekMessage(&msg, RawInput::rawInputHwnd, 0U, 0U, PM_REMOVE))
		//if (GetMessage(&msg, RawInput::rawInputHwnd, 0U, 0U))
		if (GetMessage(&msg, RawInput::rawInputHwnd, WM_INPUT, WM_INPUT))
		{
			// if (msg.message == WM_INPUT)
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
	unsigned int numDevices = 0;
	GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));
	auto deviceArray = std::make_unique<RAWINPUTDEVICELIST[]>(numDevices);
	GetRawInputDeviceList(&deviceArray[0], &numDevices, sizeof(RAWINPUTDEVICELIST));

	const auto oldKbCount = rawInputState.keyboardHandles.size();
	const auto oldMouseCount = rawInputState.mouseHandles.size();
	
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

	// Add the new devices to the deselected list
	for (const auto mouse : rawInputState.mouseHandles)
	{
		if (std::find(rawInputState.selectedMouseHandles.begin(), rawInputState.selectedMouseHandles.end(), mouse) == rawInputState.selectedMouseHandles.end()
			&& std::find(rawInputState.deselectedMouseHandles.begin(), rawInputState.deselectedMouseHandles.end(), mouse) == rawInputState.deselectedMouseHandles.end())
		{
			rawInputState.deselectedMouseHandles.push_back(mouse);
		}
	}
	for (const auto keyboard : rawInputState.keyboardHandles)
	{
		if (std::find(rawInputState.selectedKeyboardHandles.begin(), rawInputState.selectedKeyboardHandles.end(), keyboard) == rawInputState.selectedKeyboardHandles.end()
			&& std::find(rawInputState.deselectedKeyboardHandles.begin(), rawInputState.deselectedKeyboardHandles.end(), keyboard) == rawInputState.deselectedKeyboardHandles.end())
		{
			rawInputState.deselectedKeyboardHandles.push_back(keyboard);
		}
	}

	// If any devices are unplugged, remove them
	for (int i = rawInputState.selectedMouseHandles.size() - 1; i >= 0; --i)
	{
		if (std::find(rawInputState.mouseHandles.begin(), rawInputState.mouseHandles.end(), rawInputState.selectedMouseHandles[i]) == rawInputState.mouseHandles.end())
			rawInputState.selectedMouseHandles.erase(rawInputState.selectedMouseHandles.begin() + i);
	}
	for (int i = rawInputState.deselectedMouseHandles.size() - 1; i >= 0; --i)
	{
		if (std::find(rawInputState.mouseHandles.begin(), rawInputState.mouseHandles.end(), rawInputState.deselectedMouseHandles[i]) == rawInputState.mouseHandles.end())
			rawInputState.deselectedMouseHandles.erase(rawInputState.deselectedMouseHandles.begin() + i);
	}
	
	for (int i = rawInputState.selectedKeyboardHandles.size() - 1; i >= 0; --i)
	{
		if (std::find(rawInputState.keyboardHandles.begin(), rawInputState.keyboardHandles.end(), rawInputState.selectedKeyboardHandles[i]) == rawInputState.keyboardHandles.end())
			rawInputState.selectedMouseHandles.erase(rawInputState.selectedMouseHandles.begin() + i);
	}
	for (int i = rawInputState.deselectedKeyboardHandles.size() - 1; i >= 0; --i)
	{
		if (std::find(rawInputState.keyboardHandles.begin(), rawInputState.keyboardHandles.end(), rawInputState.deselectedKeyboardHandles[i]) == rawInputState.keyboardHandles.end())
			rawInputState.deselectedKeyboardHandles.erase(rawInputState.deselectedKeyboardHandles.begin() + i);
	}
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

	//TODO: use a mutex or something (this holds up the UI from updating). (Although it isn't really that important)
	do
	{
		if (rawInputHwnd == nullptr)
		{
			printf("Trying to register for raw input, but window hasn't opened yet. Waiting 0.5s\n");
			Sleep(500);
		}
	}
	while (tryCount++ < 20);

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
