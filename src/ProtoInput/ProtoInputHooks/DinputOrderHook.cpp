#include "DinputOrderHook.h"
#include <dinput.h>
#include <imgui.h>

namespace Proto
{

int createDeviceHook8WCounter = 0;
int createDeviceHook8ACounter = 0;
int createDeviceHook7WCounter = 0;
int createDeviceHook7ACounter = 0;

IDirectInput7W* pDinput7W;
// IDirectInputDevice7W* pCtrlr7W;

IDirectInput7A* pDinput7A;
// IDirectInputDevice7A* pCtrlr7A;

IDirectInput8W* pDinput8W;
// IDirectInputDevice8W* pCtrlr8W;

IDirectInput8A* pDinput8A;
// IDirectInputDevice8A* pCtrlr8A;

bool blockDinput = true;
GUID selectedControllerGuid{};

void DinputOrderHook::SetControllerGuid(const GUID& guid)
{
	selectedControllerGuid = guid;
	blockDinput = false;
}

HRESULT __stdcall Hook_DinputCreateDevice8W(IDirectInput8W* pdin, const GUID & rguid, 
										   LPDIRECTINPUTDEVICE8W* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
{
	createDeviceHook8WCounter++;
		
	return blockDinput ? 
	 	DIERR_INVALIDPARAM : //pretend it failed
	 	pdin->CreateDevice(selectedControllerGuid, lplpDirectInputDevice, pUnkOuter);
}

HRESULT __stdcall Hook_DinputCreateDevice8A(IDirectInput8A* pdin, const GUID& rguid,
										   LPDIRECTINPUTDEVICE8A* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
{
	createDeviceHook8ACounter++;

	return blockDinput ?
		DIERR_INVALIDPARAM : //pretend it failed
		pdin->CreateDevice(selectedControllerGuid, lplpDirectInputDevice, pUnkOuter);
}

HRESULT __stdcall Hook_DinputCreateDevice7W(IDirectInput7W* pdin, LPDIRECTINPUTDEVICEW* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
{
	createDeviceHook7WCounter++;

	return blockDinput ?
		DIERR_INVALIDPARAM : //pretend it failed
		pdin->CreateDevice(selectedControllerGuid, lplpDirectInputDevice, pUnkOuter);
}

HRESULT __stdcall Hook_DinputCreateDevice7A(IDirectInput7A* pdin, LPDIRECTINPUTDEVICEA* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
{
	createDeviceHook7ACounter++;

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

	ImGui::Separator();
	
	ImGui::Text("Counters:\nCreateDevice8W: %d\nCreateDevice8A: %d\nCreateDevice7W: %d\nCreateDevice7A: %d\n",
				createDeviceHook8WCounter, createDeviceHook8ACounter, createDeviceHook7WCounter, createDeviceHook7ACounter);
}

void DinputOrderHook::InstallImpl()
{		
	// typedef HRESULT(__stdcall* DirectInputCreate_t)(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter);
	// auto DirectInputCreateA = (DirectInputCreate_t)GetProcAddress(dinputHmod, "DirectInputCreateA");
	
	const auto DirectInputCreateEx = reinterpret_cast
		<HRESULT(__stdcall *)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter)>
		(GetProcAddress(LoadLibraryW(L"dinput.dll"), "DirectInputCreateEx"));

	const auto currentMod = GetModuleHandleW(0);
	
	// Dinput 8
	DirectInput8Create(currentMod, 0x0800, IID_IDirectInput8W, (void**)&pDinput8W, nullptr);
	DirectInput8Create(currentMod, 0x0800, IID_IDirectInput8A, (void**)&pDinput8A, nullptr);

	// Dinput 7
	DirectInputCreateEx(currentMod, 0x0700, IID_IDirectInput7W, (void**)&pDinput7W, nullptr);
	DirectInputCreateEx(currentMod, 0x0700, IID_IDirectInput7A, (void**)&pDinput7A, nullptr);

	
	// HRESULT dinput8DeviceReturn = pDinput8->CreateDevice(controller_guid, &pCtrlr8, nullptr);

	
	// pDinput7->CreateDevice(controller_guid, &pCtrlr7, nullptr);
	//
	// = pCtrlr7->SetDataFormat(GetdfDIJoystick());
	// dinput_device_data_format7 = 1;
	// pCtrlr7->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	//
	// pCtrlr7->EnumObjects(&DIEnumDeviceObjectsCallback7, pCtrlr7, DIDFT_AXIS);
	// pCtrlr7->Acquire();
	
	//https://kaisar-haque.blogspot.com/2008/07/c-accessing-virtual-table.html
	
	intptr_t* vptrDinput8W = *(intptr_t**)pDinput8W;
	intptr_t* vptrDinput8A = *(intptr_t**)pDinput8A;

	intptr_t* vptrDinput7W = *(intptr_t**)pDinput7W;
	intptr_t* vptrDinput7A = *(intptr_t**)pDinput7A;

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

	printf("Create device 8W: %lld\n", vptrDinput8W[3]);
	printf("Create device 8A: %lld\n", vptrDinput8A[3]);
	printf("Create device 7W: %lld\n", vptrDinput7W[3]);
	printf("Create device 7A: %lld\n", vptrDinput7A[3]);
	
	hookInfos.push_back(std::get<1>(InstallHook((void*)vptrDinput8W[3], Hook_DinputCreateDevice8W)));
	hookInfos.push_back(std::get<1>(InstallHook((void*)vptrDinput8A[3], Hook_DinputCreateDevice8A)));
	hookInfos.push_back(std::get<1>(InstallHook((void*)vptrDinput7W[3], Hook_DinputCreateDevice7W)));
	hookInfos.push_back(std::get<1>(InstallHook((void*)vptrDinput7A[3], Hook_DinputCreateDevice7A)));
}

void DinputOrderHook::UninstallImpl()
{
	for (auto& handle : hookInfos)
	{
		UninstallHook(&handle);
	}
	hookInfos.clear();
}

}
