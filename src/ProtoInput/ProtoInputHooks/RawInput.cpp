#include "RawInput.h"
#include "Gui.h"
#include <cassert>
#include <windows.h>
#include <iostream>
#include <vector>

namespace Proto
{

RawInputState RawInput::rawInputState{};

void RawInput::RefreshDevices()
{
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
	// IIRC, you can only have one raw input window for the whole process. We don't actually need to register here.
	{
	// RAWINPUTDEVICE Rid[2];
	//
	// Rid[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
	// Rid[0].usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
	// Rid[0].dwFlags = 0;    
	// Rid[0].hwndTarget = 0;
	//
	// Rid[1].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
	// Rid[1].usUsage = 0x06;              // HID_USAGE_GENERIC_KEYBOARD
	// Rid[1].dwFlags = 0;    
	// Rid[1].hwndTarget = 0;

	// if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE)
	// {
	// 	std::cerr << "Raw input failed, Error " << GetLastError() << std::endl;
	// 	return;
	// }
	//
	// std::cout << "Registered raw input devices" << std::endl;
	}
		
	RefreshDevices();
}

}
