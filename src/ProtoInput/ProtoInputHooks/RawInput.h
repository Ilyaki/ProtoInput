#pragma once
#include <vector>
#include <windows.h>
#include <bitset>
#include <hidusage.h>

namespace Proto
{

struct RawInputState
{
	// void* currentKeyboardHandle { INVALID_HANDLE_VALUE };
	// void* currentMouseHandle { INVALID_HANDLE_VALUE };
	//
	// size_t currentKeyboardIndex = -1;
	// size_t currentMouseIndex = -1;

	std::vector<void*> selectedKeyboardHandles{};
	std::vector<void*> deselectedKeyboardHandles{};
	std::vector<void*> selectedMouseHandles{};
	std::vector<void*> deselectedMouseHandles{};
	
	std::vector<void*> keyboardHandles{};
	std::vector<void*> mouseHandles{};
};

const size_t RawInputBufferSize = 1024;

class RawInput
{
private:
	static std::bitset<9> usages;
	static std::vector<HWND> forwardingWindows;

	static const std::vector<USAGE> usageTypesOfInterest;

	static void ProcessMouseInput(const RAWMOUSE& data, HANDLE deviceHandle);
	static void ProcessKeyboardInput(const RAWKEYBOARD& data, HANDLE deviceHandle);

public:
	static RawInputState rawInputState;
	static HWND rawInputHwnd;
	static bool forwardRawInput;

	static std::vector<RAWINPUT> rawinputs;
	static RAWINPUT inputBuffer[RawInputBufferSize];

	static void RefreshDevices();
		
	static void AddWindowToForward(HWND hwnd);
	static void SetUsageBitField(std::bitset<9> _usages);
	static std::bitset<9> GetUsageBitField();

	static void ProcessRawInput(HRAWINPUT rawInputHandle, bool inForeground, const MSG& msg);
		
	static void InitialiseRawInput();

	static void UnregisterGameFromRawInput();
	static void RegisterProtoForRawInput();
};

}
