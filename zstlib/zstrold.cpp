#include "stdafx.h"
#include "zstrold.h"

#include <stdio.h>							//for va_start
#include <stdarg.h>							//for va_start

#include <algorithm>						//for transform

#include <zstlib/zDebug.h>


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//------------------------------------------------------------------------------
namespace zstrold
{
	char m_Buf[BUF_LEN_FOR_ITOA];
};


//------------------------------------------------------------------------------
std::string zstrold::fmt(const char *_sFormat, ...)
{
	va_list args;
	va_start( args, _sFormat );

	string ret;
	int len = _vscprintf( _sFormat, args ) + 1;
	if(len > 1)
	{
		struct ZSTR_FMT{char *p;int len;ZSTR_FMT():p(0),len(0){}; ~ZSTR_FMT(){if(p)delete p;};};
		static ZSTR_FMT buf;
		if(len > buf.len)
		{
			delete buf.p;
			buf.len = len;
			buf.p = new char[len];
		}
		if(vsprintf_s(buf.p, buf.len, _sFormat, args) >= 0)
			ret = buf.p;
		va_end(args);
	}

	return ret;
}


//------------------------------------------------------------------------------
std::wstring zstrold::wfmt(const wchar_t *_wsFormat, ...)
{
	va_list args;
	va_start(args, _wsFormat);

	std::wstring ret;
	int len = _vscwprintf(_wsFormat, args) + 2;
	if(len > 2)
	{
		struct ZSTR_FMT{wchar_t *p;int len;ZSTR_FMT():p(0),len(0){}; ~ZSTR_FMT(){if(p)delete p;};};
		static ZSTR_FMT buf;
		if(len > buf.len)
		{
			delete buf.p;
			buf.len = len;
			buf.p = new wchar_t[len];
		}
		if(vswprintf_s(buf.p, buf.len, _wsFormat, args) >= 0)
			ret = buf.p;
		va_end(args);
	}

	return ret;
}


//------------------------------------------------------------------------------
/*std::wstring zstrold::s_to_w(const std::string& s)
{
}*/


//------------------------------------------------------------------------------
/*std::string zstrold::w_to_s(const std::wstring& s)
{
}*/


//------------------------------------------------------------------------------
void zstrold::ToUpper(string &str)
{
	std::transform(str.begin(), str.end(), str.begin(), toupper);
}


//------------------------------------------------------------------------------
std::string zstrold::ToUpper2(const string &str)
{
	string buf = str;
	ToUpper(buf);
	return buf;
}


//------------------------------------------------------------------------------
bool zstrold::HaveSubstrNoCase(const char *_str, const char *_substr)
{
	string str(_str);
	string substr(_substr);
	ToUpper(str);
	ToUpper(substr);
	//
	return (strstr(str.c_str(), substr.c_str()) != 0);
}


//------------------------------------------------------------------------------
bool zstrold::CompareNoCase(const char *str, const char *str2)
{
	return (_stricmp(str, str2) == 0);
}


//------------------------------------------------------------------------------
bool zstrold::IsFirst( const char *str_short, const char *str_long )
{
	return !strncmp( str_short, str_long, strlen( str_short ) );
}


//------------------------------------------------------------------------------
std::string zstrold::ConvertPath(const string &str)
{
	string ret(str);
	size_t i;
	while( (i = ret.find_first_of('\\')) != string::npos)
		ret[i] = '/';
	//
	return ret;
}


//------------------------------------------------------------------------------
void zstrold::trim(string& str)
{
	string::size_type pos = str.find_last_not_of(" \t\n");
	if(pos != string::npos)
	{
		str.erase(pos + 1);
		pos = str.find_first_not_of(" \t\n");
		if(pos != string::npos)
			str.erase(0, pos);
	}
	else
		str.erase(str.begin(), str.end());
}


//------------------------------------------------------------------------------
void zstrold::trimLast(string& str)
{
	string::size_type pos = str.find_last_not_of(" \t\n");
	if(pos != string::npos)
		str.erase(pos + 1);
	else
		str.erase(str.begin(), str.end());
}


//------------------------------------------------------------------------------
std::string zstrold::trim2(const string& str)
{
	string ret = str;
	trim(ret);
	return ret;
}


//------------------------------------------------------------------------------
std::string zstrold::trimLast2(const string& str)
{
	string ret = str;
	trimLast(ret);
	return ret;
}
