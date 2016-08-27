#pragma once

#include <map>

#define TCMD_CLASS TEXT("TTOTAL_CMD")

class TCmd
{
public:
	TCmd(LPCSTR szConFile);
	~TCmd();
	void setStatusText(const std::string& str);
	void sendVScroll(HWND hwnd, int downUp, int c);
	void sendKey(HWND hwnd, int keyCode);
	void processCmd(std::string & key);
private:
	HWND m_hMain;
	std::map<std::string, int> m_cmds;	// Command map in TOTALCMD.INC
	std::map<std::string, std::string> m_keyMap;	// Shortcut map set in TCmdLuancher.ini

	void sendCmds(std::string cmds, std::string cmdIndex);
};

