#pragma once
#include <cstdint>

namespace Proto
{

class HwndSelector
{	
public:
	static intptr_t selectedHwnd;

	static void UpdateMainHwnd();
};

}
