#pragma once
#include <vector>
#include <windows.h>
#include <bitset>
#include <hidusage.h>

namespace Proto
{

struct RawInputState
{
	void* currentKeyboardHandle { nullptr };
	void* currentMouseHandle { nullptr };

	size_t currentKeyboardIndex = -1;
	size_t currentMouseIndex = -1;
	
	std::vector<void*> keyboardHandles{};
	std::vector<void*> mouseHandles{};
};

const size_t RawInputBufferSize = 1024;

class RawInput
{
private:
	static void RefreshDevices();
	static std::bitset<9> usages;
	static std::vector<HWND> forwardingWindows;

	static const std::vector<USAGE> usageTypesOfInterest;


public:
	static RawInputState rawInputState;
	static HWND rawInputHwnd;
	static bool forwardRawInput;

	static std::vector<RAWINPUT> rawinputs;
	static RAWINPUT inputBuffer[RawInputBufferSize];


	static void AddWindowToForward(HWND hwnd);
	static void SetUsageBitField(std::bitset<9> _usages);
	static std::bitset<9> GetUsageBitField();

	static void ProcessRawInput(HRAWINPUT rawInputHandle, bool inForeground, const MSG& msg);
		
	static void InitialiseRawInput();

	static void UnregisterGameFromRawInput();
	static void RegisterProtoForRawInput();
};

}
