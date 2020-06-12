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
	std::map<std::string, std::string> m_keyMap;	// Shortcut map set in TCmdLuancher.ini

	void sendCmd(std::string cmd, std::string cmdIndex);
};

