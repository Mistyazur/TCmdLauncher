// Hook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Hook.h"

#include <TlHelp32.h>

#ifndef MAKEULONGLONG
#define MAKEULONGLONG(ldw, hdw) ((ULONGLONG(hdw) << 32) | ((ldw) & 0xFFFFFFFF))
#endif
#ifndef MAXULONGLONG
#define MAXULONGLONG ((ULONGLONG)~((ULONGLONG)0))
#endif

#pragma data_seg("TCmdHook")
CHAR g_szConFile[MAX_PATH] = {};
#pragma data_seg()
#pragma comment(linker,"/section:TCmdHook,rws")

extern HINSTANCE g_inst;
HHOOK g_hMsgHook = NULL;

DWORD GetMainThreadId(DWORD dwProcID)
{
	DWORD dwMainThreadID = 0;
	ULONGLONG ullMinCreateTime = MAXULONGLONG;

	HANDLE hThreadSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap != INVALID_HANDLE_VALUE) {
		THREADENTRY32 th32;
		th32.dwSize = sizeof(THREADENTRY32);
		BOOL bOK = TRUE;
		for (bOK = ::Thread32First(hThreadSnap, &th32); bOK; bOK = ::Thread32Next(hThreadSnap, &th32)) 
		{
			if (th32.th32OwnerProcessID == dwProcID) 
			{
				HANDLE hThread = ::OpenThread(THREAD_QUERY_INFORMATION, TRUE, th32.th32ThreadID);
				if (hThread) 
				{
					FILETIME afTimes[4] = {0};
					if (::GetThreadTimes(hThread,	&afTimes[0], &afTimes[1], &afTimes[2], &afTimes[3])) 
					{
						ULONGLONG ullTest = MAKEULONGLONG(afTimes[0].dwLowDateTime,	afTimes[0].dwHighDateTime);
						if (ullTest && ullTest < ullMinCreateTime) 
						{
							ullMinCreateTime = ullTest;
							dwMainThreadID = th32.th32ThreadID; // let it be main... :)
						}
					}
					::CloseHandle(hThread);
				}
			}
		}
		::CloseHandle(hThreadSnap);
	}

	return dwMainThreadID;
}

LRESULT WINAPI MsgHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		static TCmd tcmd(g_szConFile);
		static std::string key;

		MSG *pMsg = (MSG*)lParam;
		if (pMsg->message == WM_KEYDOWN)
		{
			// Clear key after switching window
			if ((pMsg->wParam == VK_TAB) || (pMsg->wParam == VK_ESCAPE))
				key.clear();
		}
		else if (pMsg->message == WM_CHAR)
		{
			static TCHAR szClass[MAX_PATH] = {};
			::GetClassName(pMsg->hwnd, szClass, MAX_PATH);
			BLW(Trace) << pMsg->hwnd << " " << pMsg->message << " " << pMsg->wParam << " " << pMsg->lParam << " " << szClass;

			if (pMsg->lParam & 0x10000000)
			{
				// 31 bit of lParam: The transition state.The value is 1 if the key is being released, or it is 0 if the key is being pressed.
				// Means it's a pop up window
			}
			else
			{
				if (_tcscmp(szClass, _T("TLister")) == 0)
				{
					if (key.empty() && pMsg->wParam == 'j')
					{
						tcmd.sendVScroll(pMsg->hwnd, 1, 6);
					}
					else if (key.empty() && pMsg->wParam == 'k')
					{
						tcmd.sendVScroll(pMsg->hwnd, 0, 6);
					}
				}
				else if (_tcscmp(szClass, _T("TMyListBox")) == 0)
				{
					if (key.empty() && pMsg->wParam == 'j')
					{
						tcmd.sendKey(pMsg->hwnd, VK_DOWN);
					}
					else if (key.empty() && pMsg->wParam == 'k')
					{
						tcmd.sendKey(pMsg->hwnd, VK_UP);
					}
					else if ((0x20 < pMsg->wParam) && (pMsg->wParam < 0x7F))
					{
						key += (CHAR)pMsg->wParam;
						tcmd.processKey(key);
					}

					// Reset key to null
					pMsg->wParam = NULL;
				}
			}
		}
		tcmd.setStatusText(key);
	}
	return CallNextHookEx(g_hMsgHook, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) BOOL InstallHook(DWORD dwPID, LPCSTR szConFile)
{
	DWORD dwThreadId;

	dwThreadId = GetMainThreadId(dwPID);
	if (dwThreadId > 0)
		g_hMsgHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)MsgHook, g_inst, dwThreadId);

	strcpy_s(g_szConFile, szConFile);

	return (g_hMsgHook != NULL);
}

extern "C" __declspec(dllexport) BOOL UninstallHook()
{
	return ((g_hMsgHook == NULL) || UnhookWindowsHookEx(g_hMsgHook));
}
