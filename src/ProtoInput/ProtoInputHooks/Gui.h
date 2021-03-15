#pragma once
#include <cstdint>

namespace Proto
{

extern unsigned long GuiThreadID;
extern intptr_t ConsoleHwnd;

int ShowGuiImpl();

void RenderImgui();

void ToggleWindow();
void SetWindowVisible(bool visible);

void ToggleConsole();
void SetConsoleVisible(bool visible);

}
