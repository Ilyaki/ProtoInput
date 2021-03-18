#pragma once

namespace Proto
{

struct FakeMouseState
{
	int x, y;
};

class FakeMouse
{
	static FakeMouseState state;
	
public:
	static const FakeMouseState& GetState() { return state; }
	static void AddMouseDelta(int dx, int dy);
	static void SetMousePos(int x, int y);
};

}