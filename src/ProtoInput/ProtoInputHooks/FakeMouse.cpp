#include "FakeMouse.h"
#include "HwndSelector.h"

namespace Proto
{

FakeMouseState FakeMouse::state{};

void FakeMouse::AddMouseDelta(int dx, int dy)
{
	state.x += dx;
	state.y += dy;

	if (state.x < 0)
		state.x = 0;
	if (state.y < 0)
		state.y = 0;

	if (state.x > HwndSelector::windowWidth)
		state.x = HwndSelector::windowWidth;

	if (state.y > HwndSelector::windowHeight)
		state.y = HwndSelector::windowHeight;
}

void FakeMouse::SetMousePos(int x, int y)
{
	state.x = x;
	state.y = y;

	if (state.x < 0)
		state.x = 0;
	if (state.y < 0)
		state.y = 0;

	if (state.x > HwndSelector::windowWidth)
		state.x = HwndSelector::windowWidth;

	if (state.y > HwndSelector::windowHeight)
		state.y = HwndSelector::windowHeight;
}

}
