#include "DinputOrderHook.h"
#include <dinput.h>
#include <imgui.h>

namespace Proto
{

// LPDIRECTINPUT7 pDinput7;
// LPDIRECTINPUTDEVICE pCtrlr7;

IDirectInput8W* pDinput8W;
// IDirectInputDevice8W* pCtrlr8W;

// IDirectInput8A* pDinput8A;
// IDirectInputDevice8A* pCtrlr8A;

bool blockDinput = true;
GUID selectedControllerGuid{};

// I think (should probably check) that the code for A/W is the same, so don't need to actually hook it twice

void DinputOrderHook::SetControllerGuid(const GUID& guid)
{
	selectedControllerGuid = guid;
	blockDinput = false;
}

HRESULT __stdcall Hook_DinputCreateDeviceW(IDirectInput8W* pdin, const GUID & rguid, LPDIRECTINPUTDEVICE8W* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
{
	std::cout << "Dinput CreateDeviceHook called\n";

	// return DIERR_INVALIDPARAM;
	
	return blockDinput ? 
	 	DIERR_INVALIDPARAM : //pretend it failed
	 	pdin->CreateDevice(selectedControllerGuid, lplpDirectInputDevice, pUnkOuter);
}

void DinputOrderHook::ShowGuiStatus()
{
	if (!blockDinput)
	{
		wchar_t* guidString;
		if (StringFromCLSID(selectedControllerGuid, &guidString) == S_OK)
			ImGui::Text("Selected GUID: %ws", guidString);
		else
			ImGui::Text("Selected GUID: ???");
	}
	else
	{
		ImGui::Text("No selected GUID");
	}
}

void DinputOrderHook::InstallImpl()
{
	const auto dinputHmod = LoadLibraryW(L"dinput.dll");
		
	// typedef HRESULT(__stdcall* DirectInputCreate_t)(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter);
	// auto DirectInputCreateA = (DirectInputCreate_t)GetProcAddress(dinputHmod, "DirectInputCreateA");
	//
	typedef HRESULT(__stdcall* DirectInputCreateEx_t)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
	auto DirectInputCreateEx = (DirectInputCreateEx_t)GetProcAddress(dinputHmod, "DirectInputCreateEx");


	// Dinput 8
	HRESULT dinput8WCreateReturn = DirectInput8Create(GetModuleHandleW(0), DIRECTINPUT_VERSION, IID_IDirectInput8W, (void**)&pDinput8W, nullptr);

	
	// HRESULT dinput8DeviceReturn = pDinput8->CreateDevice(controller_guid, &pCtrlr8, nullptr);

	
	// Dinput 7
	// DirectInputCreateEx(GetModuleHandleW(0), 0x0700, IID_IDirectInput7, (void**)&pDinput7, nullptr);
	//
	// pDinput7->CreateDevice(controller_guid, &pCtrlr7, nullptr);
	//
	// = pCtrlr7->SetDataFormat(GetdfDIJoystick());
	// dinput_device_data_format7 = 1;
	// pCtrlr7->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	//
	// pCtrlr7->EnumObjects(&DIEnumDeviceObjectsCallback7, pCtrlr7, DIDFT_AXIS);
	// pCtrlr7->Acquire();
	
	//https://kaisar-haque.blogspot.com/2008/07/c-accessing-virtual-table.html
	// intptr_t* vptr_device = *(intptr_t**)pCtrlr8W;
	intptr_t* vptr_dinputW = *(intptr_t**)pDinput8W;
	
	// intptr_t* vptr_device7 = *(intptr_t**)pCtrlr7;
	// intptr_t* vptr_dinput7 = *(intptr_t**)pDinput7;

	// using GetDeviceStateFunc = HRESULT(__stdcall*)(DWORD, LPVOID);
	// GetDeviceStateFunc GetDeviceStatePointer = (GetDeviceStateFunc)vptr_device[9];
	// installDinputHook(GetDeviceStatePointer, Dinput_GetDeviceState_Hook, "GetDeviceState");
	//
	// installDinputHook((void*)(vptr_device7[9]), Dinput_GetDeviceState_Hook7, "GetDeviceState (dinput 7)");
	//
	//
	// using GetDeviceDataFunc = HRESULT(__stdcall*)(DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
	// GetDeviceDataFunc GetDeviceDataPointer = (GetDeviceDataFunc)vptr_device[10];
	// installDinputHook(GetDeviceDataPointer, Dinput_GetDeviceData_Hook, "GetDeviceData");

	// using CreateDeviceFunc = HRESULT(__stdcall*)(REFGUID rguid, LPDIRECTINPUTDEVICE8W* lplpDirectInputDevice, LPUNKNOWN pUnkOuter);
	// CreateDeviceFunc CreateDevicePointer = (CreateDeviceFunc)vptr_dinputW[3];
	
	// installDinputHook(CreateDevicePointer, Dinput_CreateDevice_Hook, "CreateDevice");

	hookInfo = std::get<1>(InstallHook((void*)vptr_dinputW[3], Hook_DinputCreateDeviceW));
}

void DinputOrderHook::UninstallImpl()
{
	UninstallHook(&hookInfo);
}

}
