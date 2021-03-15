#include "Gui.h"
#include "imgui.h"
#include <cstdio>
#include "RawInput.h"
#include <iostream>
#include "Windows.h"
#include "HookManager.h"

namespace Proto
{

void HooksMenu()
{
    const auto& hooks = HookManager::GetHooks();
    
    static int selected = 0;
    
    ImGui::BeginChild("left pane", ImVec2(180, 0), true);

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

    	// ImGui::SameLine();
        if (ImGui::Button("Enable") && !hook->IsInstalled())
        {
            hook->Install();
        }

        ImGui::SameLine();

        if (ImGui::Button("Disable") && hook->IsInstalled())
        {
            hook->Uninstall();
        }
    	
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
    	
        ImGui::EndGroup();
    }
}

void RawInputMenu()
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

                printf("Selected raw input keyboard handle %d = 0x%X\n", 
                       RawInput::rawInputState.currentKeyboardHandle, RawInput::rawInputState.currentKeyboardHandle);
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
                
                printf("Selected raw input mouse handle %d = 0x%X\n",
                       RawInput::rawInputState.currentMouseHandle, RawInput::rawInputState.currentMouseHandle);
            }
        }
        ImGui::TreePop();
    }

    if (ImGui::Button("Refresh devices"))
    {
        RawInput::RefreshDevices();
    }
}

void RenderImgui()
{
    const auto displaySize = ImGui::GetIO().DisplaySize;
	
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(displaySize.x, displaySize.y), ImGuiCond_Always);

    if (ImGui::Begin("Main", nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     //ImGuiWindowFlags_NoBackground |
                     // ImGuiWindowFlags_MenuBar |
                     ImGuiWindowFlags_NoBringToFrontOnFocus
    ))
    {

        const ImVec2 mainWindowSize = ImGui::GetWindowSize();

        ImGui::SetNextWindowSizeConstraints(ImVec2(200, displaySize.y), ImVec2(displaySize.x - 200, displaySize.y));
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(displaySize.x / 2, 0.0f), ImGuiCond_Once);

        ImGui::Begin("Hooks", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        const auto hooksWindowSize = ImVec2(ImGui::GetWindowSize().x, mainWindowSize.y);
        ImGui::SetWindowSize(hooksWindowSize);
        HooksMenu();
        ImGui::End();


        ImGui::SetNextWindowPos(ImVec2(hooksWindowSize.x, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSizeConstraints(ImVec2(200, displaySize.y), ImVec2(displaySize.x - 200, displaySize.y));
        ImGui::SetNextWindowSize(ImVec2(displaySize.x - hooksWindowSize.x, 0.0f), ImGuiCond_Always);
    	
        ImGui::Begin("Other", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        const auto otherWindowSize = ImVec2(ImGui::GetWindowSize().x, mainWindowSize.y);
        ImGui::SetWindowSize(otherWindowSize);
        const auto otherWindowPos = ImGui::GetWindowPos();

        if (ImGui::CollapsingHeader("Raw Input"))
        {
            RawInputMenu();
        }

        ImGui::End();

    }
    ImGui::End();
}

}
