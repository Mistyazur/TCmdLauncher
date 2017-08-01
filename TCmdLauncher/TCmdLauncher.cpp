// TCmdLauncher.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include "../Hook/Hook.h"
#include "../include/Strings.h"
#include "../include/BoostLog.h"

#include "SingleProc.h"
#include "Launcher.h"
#include "TCmdLauncher.h"

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <Shlobj.h>

#ifndef _M_X64
#pragma comment(lib, "../Release/Hook32.lib")
#else
#pragma comment(lib, "../x64/Release/Hook64.lib")
#endif // !_M_X64

using namespace std;
using namespace boost;

namespace fs = boost::filesystem;
namespace pt = boost::property_tree;

#define LAUNCHER_NAME	TEXT("TCmdLauncher")
#define APP_GROUP_ID	TEXT("TOTALCMD")

TCmdLauncher::TCmdLauncher()
{

}

TCmdLauncher::~TCmdLauncher()
{

}

BOOL TCmdLauncher::exec()
{

	// Set app id for this process
	Launcher::setAppId(APP_GROUP_ID);

	// Read log settings
	const string configFile("TCmdLauncher.ini");
	if (!fs::exists(configFile))
		return FALSE;

	// Get settings
	pt::wptree tree;
	pt::read_ini(configFile, tree);
	bool bAutoFreeCheck = tree.get<bool>(TEXT("TotalCommander.AutoFreeCheck"), false);

#ifndef _M_X64
	wstring tcmdExeFile = tree.get<wstring>(TEXT("TotalCommander.Exe32File"), TEXT(""));
#else
	wstring tcmdExeFile = tree.get<wstring>(TEXT("TotalCommander.Exe64File"), TEXT(""));
#endif
	wstring tcmdConFile = tree.get<wstring>(TEXT("TotalCommander.ConFile"), TEXT(""));
	wstring tcmdFtpFile = tree.get<wstring>(TEXT("TotalCommander.FtpFile"), TEXT(""));

	// Run Total commander
	wstring cmd = (wformat(_T("%s /I=%s /F=%s"))
				   % fs::absolute(tcmdExeFile).c_str()
				   % fs::absolute(tcmdConFile).c_str()
				   % fs::absolute(tcmdFtpFile).c_str()).str();
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION pi;
	if (::CreateProcess(NULL, (LPWSTR)cmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		// Get hwnd of total commander
		HWND hTCmd;
		do {
			hTCmd = ::FindWindow(TCMD_CLASS, NULL);
		} while (NULL == hTCmd);

		// Set task bar group
		Launcher::setWndAppID(hTCmd, APP_GROUP_ID);

		// Check
		if (bAutoFreeCheck)
			autoFreeUserCheck();

		// Install hook
		InstallHook(pi.dwProcessId, fs::absolute(configFile).string().c_str());

		// Wait until process exit
		::WaitForSingleObject(pi.hProcess, INFINITE);

		// Reset
		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);

		// If Total Commander was pinned to task bar, 
		// change the link target path to this launcher.

		TCHAR szPath[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath))) {

			std::wstring taskBarPath(szPath);
			taskBarPath += TEXT("\\Microsoft\\Internet Explorer\\Quick Launch\\User Pinned\\TaskBar");

			fs::path fullpath(taskBarPath, fs::native);
			if (!fs::exists(taskBarPath))
				return FALSE;

			fs::recursive_directory_iterator end_iter;
			for (fs::recursive_directory_iterator iter(fullpath); iter != end_iter; iter++) {
				try {
					if (!fs::is_directory(*iter)) {
						if (Launcher::getLnkAppPath(iter->path().wstring().c_str(), szPath, MAX_PATH)) {
							fs::path targetPath(szPath);
							if (boost::iequals(targetPath.filename().wstring(), fs::path(tcmdExeFile).filename().wstring())) {
								if (::GetModuleFileName(NULL, szPath, MAX_PATH) > 0)
									Launcher::setLnkAppPath(iter->path().wstring().c_str(), szPath);

								break;
							}
						}
					}
				}
				catch (const std::exception & ex) {
					continue;
				}
			}
		}

		return TRUE;
	}

	return FALSE;
}

VOID TCmdLauncher::autoFreeUserCheck()
{
	HWND hFreeCheck;
	HWND hFreeCheckLabel;
	TCHAR szInfo[16];

	do {
		hFreeCheck = ::FindWindow(_T("TNASTYNAGSCREEN"), NULL);
	} while (NULL == hFreeCheck);

	while (TRUE) {
		hFreeCheckLabel = hFreeCheck;
		for (int i = 0; i < 4; ++i)
			hFreeCheckLabel = ::GetWindow(hFreeCheckLabel, GW_CHILD);

		::GetWindowText(hFreeCheckLabel, szInfo, 16);
		if (szInfo[0] >= '1' && szInfo[0] <= '3')
			break;
		else
			::Sleep(100);
	}

	UINT nKey = (UINT)szInfo[0];
	::PostMessage(hFreeCheck, WM_KEYDOWN, nKey, (0x00 << 24) | (MapVirtualKey(nKey, 0) << 16) | 0x0001);
	::Sleep(1);
	::SendMessage(hFreeCheck, WM_KEYUP, nKey, (0xC0 << 24) | (MapVirtualKey(nKey, 0) << 16) | 0x0001);
	::Sleep(1);
}

BOOL ActivateTCMD()
{
	// Get hwnd of total commander

	HWND hTCmd = ::FindWindow(TCMD_CLASS, NULL);
	if (hTCmd == NULL)
		return FALSE;

	::ShowWindow(hTCmd, SW_HIDE);
	::Sleep(100);
	::ShowWindow(hTCmd, SW_SHOW);

	::SetForegroundWindow(hTCmd);

	return TRUE;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
					   _In_opt_ HINSTANCE hPrevInstance,
					   _In_ LPTSTR    lpCmdLine,
					   _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize launcher

	Launcher::initialize();

	// Check single process

	SingleProc sProc;
	if (sProc.exists(LAUNCHER_NAME)) {
		ActivateTCMD();
		return 0;
	}
	sProc.init(LAUNCHER_NAME);

	// Run total commander

	TCmdLauncher tcmdLauncher;
	tcmdLauncher.exec();

	// Delete Launcher

	Launcher::uninitialize();

	return 0;
}
