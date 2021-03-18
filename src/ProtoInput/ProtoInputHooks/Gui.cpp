#include "Gui.h"
#include "imgui.h"
#include <cstdio>
#include "RawInput.h"
#include <iostream>
#include "Windows.h"
#include "HookManager.h"
#include <algorithm>
#include "MessageFilterHook.h"
#include "MessageList.h"
#include "FakeMouse.h"
#include "HwndSelector.h"

namespace Proto
{

intptr_t ConsoleHwnd;

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

template<typename T>
void HandleSelectableDualList(std::vector<T>& selected, std::vector<T>& deselected)
{
    std::sort(selected.begin(), selected.end());
    std::sort(deselected.begin(), deselected.end());
	
    std::vector<T> toAddToB{};
    std::vector<std::vector<T>::const_iterator> removeA{};
    std::vector<std::vector<T>::const_iterator> removeB{};

    for (size_t i = 0; i < selected.size(); ++i)
    {
        char buf[32];
        sprintf(buf, "Handle %d", (intptr_t)selected[i]);
        if (ImGui::Selectable(buf, true))
        {
            toAddToB.push_back(selected[i]);
            removeA.push_back(selected.begin() + i);
        }
    }
	
    for (size_t i = 0; i < deselected.size(); ++i)
    {
        char buf[32];
        sprintf(buf, "Handle %d", (intptr_t)deselected[i]);
        if (ImGui::Selectable(buf, false))
        {
            selected.push_back(deselected[i]);
            removeB.push_back(deselected.begin() + i);
        }
    }

    for (const auto x : toAddToB)
        deselected.push_back(x);
    for (const auto x : removeA)
        selected.erase(x);
    for (const auto x : removeB)
        deselected.erase(x);
}

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
        ImGui::TextWrapped("%s Hook: %s", hook->GetHookName(), hook->IsInstalled() ? "Enabled" : "Disabled");

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
    if (ImGui::TreeNode("Mouse Raw Input Handle"))
    {
        HandleSelectableDualList(RawInput::rawInputState.selectedMouseHandles, RawInput::rawInputState.deselectedMouseHandles);
    	
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Keyboard Raw Input Handle"))
    {
        HandleSelectableDualList(RawInput::rawInputState.selectedKeyboardHandles, RawInput::rawInputState.deselectedKeyboardHandles);

        ImGui::TreePop();
    }

    if (ImGui::Button("Refresh devices"))
    {
        RawInput::RefreshDevices();
    }
}

void ControlsMenu()
{
	if (ImGui::Button("Hide GUI"))
	{
        SetWindowVisible(false);
	}
	
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted("Press right control + right alt + 1/2/3/4/... to open the GUI for instance 1/2/3/4/...");
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }	

	if (ImGui::Button("Toggle console"))
	{
        ToggleConsole();
	}
}

void InputStatusMenu()
{
    const auto& mouseState = FakeMouse::GetState();
    ImGui::TextWrapped("Fake mouse position (%d, %d)", mouseState.x, mouseState.y);
    ImGui::TextWrapped("Window dimensions: (%d, %d)", HwndSelector::windowWidth, HwndSelector::windowHeight);
}

void RenderImgui()
{
   // ImGui::ShowDemoWindow();
   // return;
	
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
        ImGui::SetNextWindowSize(ImVec2(displaySize.x * 0.7f, 0.0f), ImGuiCond_Once);

        ImGui::Begin("Hooks/Filter", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        const auto hooksWindowSize = ImVec2(ImGui::GetWindowSize().x, mainWindowSize.y);
        ImGui::SetWindowSize(hooksWindowSize);
    	
        if (ImGui::BeginTabBar("Tabs"))
        {
            if (ImGui::BeginTabItem("Hooks"))
            {
                HooksMenu();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Message filter"))
            {
                if (ImGui::BeginTabBar("Filter tabs"))
                {
                    if (ImGui::BeginTabItem("Modify"))
                    {
                        if (!HookManager::IsInstalled(ProtoHookIDs::MessageFilterHookID))
                        {
                            ImGui::PushID(1337);
                            ImGui::PushStyleColor(ImGuiCol_Text,
                                                  (ImVec4)ImColor::HSV(35.0f / 255.0f, 0.9f, 0.9f)
                            );
                            ImGui::TextWrapped("Warning: Message Filter hook is disabled.\nMessage filtering/blocking will not work!");
                            ImGui::PopStyleColor(1);
                            ImGui::PopID();
                        }
                    	
                        MessageFilterHook::FilterGui();

                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Block"))
                    {
                        if (!HookManager::IsInstalled(ProtoHookIDs::MessageFilterHookID))
                        {
                            ImGui::PushID(1337);
                            ImGui::PushStyleColor(ImGuiCol_Text,
                                                  (ImVec4)ImColor::HSV(35.0f / 255.0f, 0.9f, 0.9f)
                            );
                            ImGui::TextWrapped("Warning: Message Filter hook is disabled.\nMessage filtering/blocking will not work!");
                            ImGui::PopStyleColor(1);
                            ImGui::PopID();
                        }
                    	
                        MessageList::ShowUI();
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    	    	
        ImGui::End();


        ImGui::SetNextWindowPos(ImVec2(hooksWindowSize.x, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSizeConstraints(ImVec2(200, displaySize.y), ImVec2(displaySize.x - 200, displaySize.y));
        ImGui::SetNextWindowSize(ImVec2(displaySize.x - hooksWindowSize.x, 0.0f), ImGuiCond_Always);
    	
        ImGui::Begin("Other", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        const auto otherWindowSize = ImVec2(ImGui::GetWindowSize().x, mainWindowSize.y);
        ImGui::SetWindowSize(otherWindowSize);
        const auto otherWindowPos = ImGui::GetWindowPos();

        if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ControlsMenu();
        }
    	
        if (ImGui::CollapsingHeader("Raw Input", ImGuiTreeNodeFlags_DefaultOpen))
        {
            RawInputMenu();
        }

        if (ImGui::CollapsingHeader("Input Status", ImGuiTreeNodeFlags_DefaultOpen))
        {
            InputStatusMenu();
        }

        ImGui::End();

    }
    ImGui::End();
}

}
