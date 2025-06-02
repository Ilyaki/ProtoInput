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
	HCURSOR hCursor;
	static constexpr auto transparencyKey = RGB(0, 0, 1);
	
	int oldX, oldY;
	bool oldHadShowCursor = true;
	//TODO: width/height probably needs to change


	// DrawFakeCursorFix. cursor offset scan and cursor size fix
	int cursoroffsetx, cursoroffsety; 
	int offsetSET; //0:sizing 1:offset 2:done
	int cursorWidth = 40;
	int cursorHeight = 40;

	// This is either on or off for a given game (ie. it doesn't change)
	bool drawingEnabled = false;

	// This changes when the hook detects SetCursor/ShowCursor
	bool showCursor = true;

	bool toggleVisilbityShorcutEnabled = false;
	unsigned int toggleVisibilityVkey = VK_HOME;
	
	void DrawCursor();
	
public:
	static FakeCursor state;
	bool DrawFakeCursorFix;
	void StartInternal();
	void StartDrawLoopInternal();

	static bool& GetToggleVisilbityShorcutEnabled()
	{
		return state.toggleVisilbityShorcutEnabled;
	}

	static unsigned int& GetToggleVisibilityVkey()
	{
		return state.toggleVisibilityVkey;
	}
	
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

	static void SetCursorVisibility(bool visible)
	{
		state.showCursor = visible;
		NotifyUpdatedCursorPosition();
	}

	static bool GetCursorVisibility()
	{
		return state.showCursor;
	}

	static void SetCursorHandle(HCURSOR newCursor)
	{
		state.hCursor = newCursor;
	}
	
	static void EnableDisableFakeCursor(bool enable);

	static void Initialise();
};

}
