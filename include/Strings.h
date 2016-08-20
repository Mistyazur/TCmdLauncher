#ifndef _STRINGS_H_
#define _STRINGS_H_

#include <memory>
#include <string>
#include <Stringapiset.h>

class AString
{
public:
	void init(int cp, const char* s)
	{
		int nLen;

		m_str = std::make_shared<std::string>(s);
		nLen = MultiByteToWideChar(cp, 0, s, -1, NULL, NULL);
		m_wstr = std::make_shared<std::wstring>(nLen + 1, 0);
		MultiByteToWideChar(cp, 0, s, -1, const_cast<LPWSTR>(m_wstr->c_str()), nLen);
	}

	void init(int cp, const wchar_t* s)
	{
		int nLen;

		m_wstr = std::make_shared<std::wstring>(s);
		nLen = WideCharToMultiByte(cp, 0, s, -1, NULL, NULL, NULL, NULL);
		m_str = std::make_shared<std::string>(nLen + 1, 0);
		WideCharToMultiByte(cp, 0, s, -1, const_cast<LPSTR>(m_str->c_str()), nLen, NULL, NULL);
	}

	const char* data()
	{
		return m_str->c_str();
	}

	const wchar_t* wdata()
	{
		return m_wstr->c_str();
	}

private:
	std::shared_ptr<std::string> m_str;
	std::shared_ptr<std::wstring> m_wstr;
};

class AcpString : public AString
{
public:
	AcpString(const char* s)
	{
		init(CP_ACP, s);
	}

	AcpString(const wchar_t* s)
	{
		init(CP_ACP, s);
	}

	AcpString(const std::string& s)
	{
		init(CP_ACP, s.c_str());
	}

	AcpString(const std::wstring& s)
	{
		init(CP_ACP, s.c_str());
	}
};

class Utf8String : public AString
{
public:
	Utf8String(const char* s)
	{
		init(CP_UTF8, s);
	}

	Utf8String(const wchar_t* s)
	{
		init(CP_UTF8, s);
	}

	Utf8String(const std::string& s)
	{
		init(CP_UTF8, s.c_str());
	}

	Utf8String(const std::wstring& s)
	{
		init(CP_UTF8, s.c_str());
	}
};

#endif // !_STRINGS_H_