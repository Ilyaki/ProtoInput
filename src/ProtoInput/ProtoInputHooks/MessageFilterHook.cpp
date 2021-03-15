#include "MessageBoxHook.h"
#include "MessageFilterHook.h"
#include "MessageFilter.h"
#include <imgui.h>

namespace Proto
{

template<typename T>
bool* IsFilterEnabled()
{
	static bool enabled = false;
	return &enabled;
}

template<typename X, typename... T>
bool MessageFilterAllow(unsigned int message, unsigned int lparam, unsigned int wparam, intptr_t hwnd)
{
	bool a = (!*IsFilterEnabled<X>() || message < X::MessageMin() || message > X::MessageMax() || X::Filter(message, lparam, wparam, hwnd));
	
	if constexpr (sizeof...(T) != 0)
		return a && MessageFilterAllow<T...>(message, lparam, wparam, hwnd);
	else
		return a;
}

inline BOOL FilterMessage(MSG* lpMsg)
{
	if (!MessageFilterAllow<PROTO_MESSAGE_FILTERS>(lpMsg->message, lpMsg->lParam, lpMsg->wParam, (intptr_t)lpMsg->hwnd))
	{
		//Massive performance benefits for returning a successful WM_NULL compared to causing an error in the application.
		memset(lpMsg, 0, sizeof(MSG));
	}
	
	return 1;
}

template<typename X, typename... T>
void SetFilter(ProtoMessageFilterIDs filterID, bool enabled)
{
	if (X::FilterID() == filterID)
	{
		*IsFilterEnabled<X>() = enabled;
		printf("Set filter %s to %s\n", X::Name(), enabled ? "enabled" : "disabled");
		return;
	}

	if constexpr (sizeof...(T) != 0)
		SetFilter<T...>(filterID, enabled);
}

void MessageFilterHook::SetFilterEnabled(ProtoMessageFilterIDs filterID, bool enabled)
{
	SetFilter<PROTO_MESSAGE_FILTERS>(filterID, enabled);
}

template<typename X, typename... T>
std::tuple<bool*, const char*, const char*> ShowListSelectable(int& selected)
{
	const int N = sizeof...(T);
	
	ImGui::PushID(N);
	ImGui::PushStyleColor(ImGuiCol_Text,
						  *IsFilterEnabled<X>() ?
						  (ImVec4)ImColor::HSV(2 / 7.0f, 0.6f, 0.6f) :
						  (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.6f)
	);

	if (ImGui::Selectable(X::Name(), selected == N))
	{
		selected = N;
	}

	ImGui::PopStyleColor(1);
	ImGui::PopID();

	auto tup1 = selected == N ?
		std::tuple<bool*, const char*, const char*>{ IsFilterEnabled<X>(), X::Name(), X::Description() } :
		std::tuple<bool*, const char*, const char*>{ nullptr, "???", "???" };
	
	if constexpr (sizeof...(T) == 0)
		return tup1;
	else
	{
		auto tup2 = ShowListSelectable<T...>(selected);
		return std::get<0>(tup1) == nullptr ? tup2 : tup1;
	}
}

void MessageFilterHook::FilterGui()
{
	ImGui::BeginChild("left pane filter", ImVec2(180, 0), true);
		
	static int selected = 0;
	auto [enabledPtr, name, desc] = ShowListSelectable<PROTO_MESSAGE_FILTERS>(selected);

	ImGui::EndChild();

	ImGui::SameLine();

	if (enabledPtr != nullptr)
	{
		ImGui::BeginGroup();
		ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
		ImGui::Text("%s Filter: %s", name, *enabledPtr ? "Enabled" : "Disabled");

		if (ImGui::Button("Enable") && !*enabledPtr)
		{
			*enabledPtr = true;
		}

		ImGui::SameLine();

		if (ImGui::Button("Disable") && *enabledPtr)
		{
			*enabledPtr = false;
		}

		ImGui::Separator();
		if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Description"))
			{
				ImGui::TextWrapped(desc);
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::EndChild();

		ImGui::EndGroup();
	}
}

BOOL WINAPI Hook_GetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
	const auto ret = GetMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);

	return ret == -1 ? -1 : FilterMessage(lpMsg);
}

BOOL WINAPI Hook_GetMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
	const auto ret = GetMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);

	return ret == -1 ? -1 : FilterMessage(lpMsg);
}

BOOL WINAPI Hook_PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	const auto ret = PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

	return ret == FALSE ? FALSE : ((1 + FilterMessage(lpMsg)) / 2);
}

BOOL WINAPI Hook_PeekMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	const auto ret = PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

	return ret == FALSE ? FALSE : ((1 + FilterMessage(lpMsg)) / 2);
}

void MessageFilterHook::InstallImpl()
{
	auto [statusGW, _hookInfoGetMessageW] = InstallNamedHook(L"user32", "GetMessageW", Hook_GetMessageW);
	this->hookInfoGetMessageW = _hookInfoGetMessageW;

	auto [statusGA, _hookInfoGetMessageA] = InstallNamedHook(L"user32", "GetMessageA", Hook_GetMessageA);
	this->hookInfoGetMessageA = _hookInfoGetMessageA;

	auto [statusPW, _hookInfoPeekMessageW] = InstallNamedHook(L"user32", "PeekMessageW", Hook_PeekMessageW);
	this->hookInfoPeekMessageW = _hookInfoPeekMessageW;

	auto [statusPA, _hookInfoPeekMessageA] = InstallNamedHook(L"user32", "PeekMessageA", Hook_PeekMessageA);
	this->hookInfoPeekMessageA = _hookInfoPeekMessageA;
}

void MessageFilterHook::UninstallImpl()
{
	UninstallHook(&hookInfoGetMessageW);
	UninstallHook(&hookInfoGetMessageA);
	UninstallHook(&hookInfoPeekMessageW);
	UninstallHook(&hookInfoPeekMessageA);
}

}