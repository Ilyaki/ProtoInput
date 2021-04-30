#include "include/protoinpututil.h"
#include <BlackBone/Process/Process.h>

std::vector<HANDLE> suspendedThreads{ };

extern "C" __declspec(dllexport) void SuspendExplorer()
{
	auto pids = blackbone::Process::EnumByName(L"explorer.exe");
	for (const auto pid : pids)
	{		
		HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);
		if (h != INVALID_HANDLE_VALUE) {
			THREADENTRY32 te;
			te.dwSize = sizeof(te);
			if (Thread32First(h, &te)) {
				do {
					if (te.th32OwnerProcessID == pid &&
						te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))
					{
						if (auto hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID); hThread != NULL)
						{
							SuspendThread(hThread);

							suspendedThreads.push_back(hThread);
						}
					}
					te.dwSize = sizeof(te);
				}
				while (Thread32Next(h, &te));
			}
			CloseHandle(h);
		}

	}
}

extern "C" __declspec(dllexport) void RestartExplorer()
{	
	for (const auto h : suspendedThreads)
	{
		ResumeThread(h);
		CloseHandle(h);
	}

	suspendedThreads.clear();


	// Just in case its a reboot of the program that's trying to clean up a crashed exit
	auto pids = blackbone::Process::EnumByName(L"explorer.exe");
	for (const auto pid : pids)
	{
		HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);
		if (h != INVALID_HANDLE_VALUE) {
			THREADENTRY32 te;
			te.dwSize = sizeof(te);
			if (Thread32First(h, &te)) {
				do {
					if (te.th32OwnerProcessID == pid &&
						te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))
					{
						if (auto hThread = OpenThread(THREAD_RESUME, FALSE, te.th32ThreadID); hThread != NULL)
						{
							ResumeThread(hThread);
							CloseHandle(hThread);
						}
					}
					te.dwSize = sizeof(te);
				}
				while (Thread32Next(h, &te));
			}
			CloseHandle(h);
		}

	}
}