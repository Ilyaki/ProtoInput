#pragma once

namespace Proto
{

extern unsigned long GuiThreadID;

int ShowGuiImpl();

void RenderImgui();

void ToggleWindow();
void SetWindowVisible(bool visible);

}