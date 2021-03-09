#pragma once
#include <vector>

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

class RawInput
{
private:
	static void RefreshDevices();
	
public:
	static RawInputState rawInputState;
	
	static void InitialiseRawInput();
};

}
