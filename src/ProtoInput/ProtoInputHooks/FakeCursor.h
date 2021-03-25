#pragma once
#include <mutex>
#include <windows.h>

namespace Proto
{

class FakeCursor
{
	std::mutex mutex{};
	std::condition_variable conditionvar{};

	HWND pointerWindow = nullptr;
	HDC hdc;
	HBRUSH transparencyBrush;
	HCURSOR hIcon;
	static constexpr auto transparencyKey = RGB(0, 0, 1);
	
	int oldX, oldY;

	bool drawingEnabled = false;
	
	void DrawCursor();
	
public:
	static FakeCursor state;

	void StartInternal();
	void StartDrawLoopInternal();
	
	static void NotifyUpdatedCursorPosition()
	{
		state.conditionvar.notify_one();
	}

	static bool IsDrawingEnabled()
	{
		return state.drawingEnabled;
	}

	static HWND GetPointerWindow()
	{
		return state.pointerWindow;
	}
	
	static void EnableDisableFakeCursor(bool enable);

	static void Initialise();
};

}
