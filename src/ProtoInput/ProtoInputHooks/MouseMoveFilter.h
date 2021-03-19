#pragma once
#include <WinUser.h>
#include "MessageFilterBase.h"
#include "protoloader.h"
#include "FakeMouseKeyboard.h"

namespace Proto
{

class MouseMoveFilter : public Proto::MessageFilterBase<ProtoMessageFilterIDs::MouseMoveFilterID, WM_MOUSEMOVE>
{
public:
	static bool Filter(unsigned int message, unsigned int* lparam, unsigned int* wparam, intptr_t hwnd)
	{
#define PROTO_MMFKEY(x, y) ((FakeMouseKeyboard::IsKeyStatePressed(x)) ? (y) : (0))
		
		*wparam = 
			PROTO_MMFKEY(VK_CONTROL, MK_CONTROL) |
			PROTO_MMFKEY(VK_SHIFT, MK_SHIFT) |
			PROTO_MMFKEY(VK_LBUTTON, MK_LBUTTON) |
			PROTO_MMFKEY(VK_MBUTTON, MK_MBUTTON) |
			PROTO_MMFKEY(VK_RBUTTON, MK_RBUTTON) |
			PROTO_MMFKEY(VK_XBUTTON1, MK_XBUTTON1) |
			PROTO_MMFKEY(VK_XBUTTON2, MK_XBUTTON2);

#undef PROTO_MMFKEY
				
		*lparam = MAKELPARAM(FakeMouseKeyboard::GetMouseState().x, FakeMouseKeyboard::GetMouseState().y);

		return true;
		
	}

	static const char* Name() { return "Mouse move"; }
	static const char* Description()
	{
		return
			"Modifies the WM_MOUSEMOVE messages to return our fake mouse state. "
			"The WM_MOUSEMOVE messages also contains information about the modifier (shift and ctrl) keys and the mouse buttons. "
			"This filter is important for mouse movement to work correctly. ";
	}
};

}
