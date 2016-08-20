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
	void processKey(std::string & key);
private:
	HWND m_hMain;
	std::map<std::string, int> m_cmds;
	std::map<std::string, std::string> m_keyMap;

	void sendCmds(std::string cmds, std::string index);
};

