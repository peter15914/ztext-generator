#pragma once

/*/----------------------------------------------------------------------------
	@namespace zstrold
	@desc
		устаревшее.
		некоторые вспомогательные функции для работы с std::string

	2007Jan03 created. zm
	2011Oct19 - moved to zutlold
-----------------------------------------------------------------------------*/

#include <string>


//------------------------------------------------------------------------------
namespace zstrold
{
	using std::string;
	//
	enum { BUF_LEN_FOR_ITOA = 128 };
	extern char m_Buf[BUF_LEN_FOR_ITOA];
	//
	inline string itoa(int value, int radix)
	{
		::_itoa_s(value, m_Buf, BUF_LEN_FOR_ITOA, radix);
		return m_Buf;
	};
	//
	string fmt(const char *_sFormat, ...);
	std::wstring wfmt(const wchar_t *_wsFormat, ...);
	//
	void ToUpper(string &str);
	string ToUpper2(const string &str);
	bool HaveSubstrNoCase(const char *str, const char *substr);
	bool CompareNoCase(const char *str, const char *str2);
	//
	bool IsFirst(const char *str_short, const char *str_long);
	//
	string ConvertPath(const string &str);
	//
	void trim(string& str);
	string trim2(const string& str);
	void trimLast(string& str);
	string trimLast2(const string& str);
	//
	//std::wstring s_to_w(const std::string& s);
	//std::string w_to_s(const std::wstring& s);
};


