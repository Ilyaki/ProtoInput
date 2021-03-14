#include "Gui.h"
#include "imgui.h"
#include <cstdio>
#include "RawInput.h"
#include <iostream>
#include "Windows.h"
#include "HookManager.h"
#include <string>

namespace Proto
{

void HooksMenu()
{
    if (ImGui::Begin("Hooks"))
    {
        const auto& hooks = HookManager::GetHooks();
    	
        static int selected = 0;
        
        ImGui::BeginChild("left pane", ImVec2(150, 0), true);

        for (int i = 0; i < hooks.size(); i++)
        {
            const auto& hook = hooks[i];
        	
            ImGui::PushID(i);
            ImGui::PushStyleColor(ImGuiCol_Text, 
                                  hook->IsInstalled() ? 
									(ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f) :
									(ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.6f)
            );

            if (ImGui::Selectable(hook->GetHookName(), selected == i))
                selected = i;

            ImGui::PopStyleColor(1);
            ImGui::PopID();
        }

        ImGui::EndChild();

        ImGui::SameLine();

    	if (!hooks.empty())
        {
            const auto& hook = hooks[selected];
    		
            ImGui::BeginGroup();
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
            ImGui::Text("%s Hook: %s", hook->GetHookName(), hook->IsInstalled() ? "Enabled" : "Disabled");
            ImGui::Separator();
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Description"))
                {
                    ImGui::TextWrapped(hook->GetHookDescription());
                    ImGui::EndTabItem();
                }
            	
                if (hook->HasGuiStatus() && ImGui::BeginTabItem("Details"))
                {
                    hook->ShowGuiStatus();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
    		
            ImGui::EndChild();
    		
            if (ImGui::Button("Enable") && !hook->IsInstalled())
            {
                hook->Install();
            }
    		
            ImGui::SameLine();
    		
            if (ImGui::Button("Disable") && hook->IsInstalled())
            {
                hook->Uninstall();
            }
    		
            ImGui::EndGroup();
        }
    }
}

void RawInputMenu()
{
    if (ImGui::Begin("Raw Input"))
    {
        if (ImGui::TreeNode("Keyboard Raw Input Handle"))
        {
            for (unsigned int n = 0; n < RawInput::rawInputState.keyboardHandles.size(); n++)
            {
                char buf[32];
                sprintf(buf, "Handle %d", (intptr_t)RawInput::rawInputState.keyboardHandles[n]);
                if (ImGui::Selectable(buf, RawInput::rawInputState.currentKeyboardIndex == n))
                {
                    RawInput::rawInputState.currentKeyboardIndex = n;
                    RawInput::rawInputState.currentKeyboardHandle = RawInput::rawInputState.keyboardHandles[n];
                    std::cout << "Selected keyboard raw input handle " << RawInput::rawInputState.currentKeyboardHandle << std::endl;
                }
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Mouse Raw Input Handle"))
        {
            for (unsigned int n = 0; n < RawInput::rawInputState.mouseHandles.size(); n++)
            {
                char buf[32];
                sprintf(buf, "Handle %d", (intptr_t)RawInput::rawInputState.mouseHandles[n]);
                if (ImGui::Selectable(buf, RawInput::rawInputState.currentMouseIndex == n))
                {
                    RawInput::rawInputState.currentMouseIndex = n;
                    RawInput::rawInputState.currentMouseHandle = RawInput::rawInputState.mouseHandles[n];
                    std::cout << "Selected mouse raw input handle " << RawInput::rawInputState.currentMouseHandle << std::endl;
                }
            }
            ImGui::TreePop();
        }

        if (ImGui::Button("Open message box"))
        {
            std::cout << "Open message box button thread = " << GetCurrentThreadId() << std::endl;
            MessageBoxW(NULL, L"Test", L"Test", MB_OK);
        }
    }
    ImGui::End();
}

void RenderImgui()
{
	ImGui::ShowDemoWindow();

	RawInputMenu();

	HooksMenu();
}

}
