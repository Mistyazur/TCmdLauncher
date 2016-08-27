#include "stdafx.h"
#include "TCmd.h"

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>

#define TCMD_INC "TOTALCMD.INC"
#define WM_TCMD	WM_USER + 0x33

using namespace boost;
namespace fs = boost::filesystem;
namespace pt = boost::property_tree;

TCmd::TCmd(LPCSTR szConFile)
{
	m_hMain = ::FindWindow(TCMD_CLASS, NULL);

	// Get commander file path
	TCHAR szTCmdPath[MAX_PATH];
	::GetModuleFileName(NULL, szTCmdPath, MAX_PATH);
	std::string cmdsFile = fs::path(szTCmdPath).parent_path().string();
	cmdsFile.append("\\");
	cmdsFile.append(TCMD_INC);

	// Read commands
	pt::ptree tree;
	pt::read_ini(cmdsFile, tree);
	for (auto b : tree)
	{
		for (auto p : b.second)
		{
			std::string& cmd = p.second.get_value<std::string>();
			m_cmds.insert(std::pair<std::string, int>(p.first, lexical_cast<int>(cmd.c_str(), cmd.find_first_of(";"))));
		}
	}

	// Read shortcuts
	pt::read_ini(szConFile, tree);
	auto it = tree.find("KeyMap");
	if (it != tree.not_found())
	{
		for (auto p : it->second)
		{
			std::string key = p.first;
			std::string& cmd = p.second.get_value<std::string>();

			if (key.find("map ", 0) == 0)
			{
				key.erase(0, 4);
				m_keyMap.insert(std::pair<std::string, std::string>(key, cmd));
			}
		}
	}
}

TCmd::~TCmd()
{
}

void TCmd::setStatusText(const std::string & keySequence)
{
	std::string status;
	if (keySequence.empty())
		status = "Normal";
	else
		status = "Insert: " + keySequence;

	::SetWindowTextA(m_hMain, status.c_str());
}

void TCmd::sendVScroll(HWND hwnd, int downUp, int c)
{
	for (int i = 0; i < c; ++i)
	{
		if (downUp == 0)
			::PostMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0);
		else
			::PostMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
	}
}

void TCmd::sendKey(HWND hwnd, int keyCode)
{
	::PostMessage(hwnd, WM_KEYDOWN, keyCode, (0x00 << 24) | (MapVirtualKey(keyCode, 0) << 16) | 0x0001);
	::Sleep(1);
	::SendMessage(hwnd, WM_KEYUP, keyCode, (0xC0 << 24) | (MapVirtualKey(keyCode, 0) << 16) | 0x0001);
	::Sleep(1);
}

void TCmd::processCmd(std::string & keySequence)
{
	std::string keySequenceWithoutID;
	regex exp("([a-zA-Z]+)([0-9]+)$");
	match_results<std::string::const_iterator> mr;

	if (regex_search(keySequence, mr, exp, match_default))
		keySequenceWithoutID = mr[1] + "?";	// ? replace index of commander in settings
	else
		keySequenceWithoutID = keySequence;

	// Check if this shortcut's valid

	bool bValid = false;

	for (auto it : m_keyMap)
	{
		if (it.first.find(keySequenceWithoutID, 0) == 0)
		{
			if (it.first == keySequenceWithoutID)	// Totally match, send command
			{
				sendCmds(it.second, mr[2]);
				keySequence.clear();
			}

			bValid = true;
			break;
		}
	}

	if (!bValid)
		keySequence.clear();
}

void TCmd::sendCmds(std::string cmds, std::string index)
{
	std::vector<std::string> vecSplit;

	split(vecSplit, cmds, is_any_of(","), token_compress_on);	// Command string may be consist by a sequence of commands separated by comma
	for (auto cmd : vecSplit)
	{
		cmd += index;
		auto iter = m_cmds.find(cmd);
		if (iter != m_cmds.end())
			::PostMessage(m_hMain, WM_TCMD, iter->second, NULL);
	}
}

