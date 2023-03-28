#include "stdafx.h"
#include "zstr.h"

#include <stdio.h>							//for va_start
#include <stdarg.h>							//for va_start

#include <algorithm>						//for transform

#include <zstlib/zDebug.h>
#include <zstlib/textutl.h>
#include <zstlib/zsubstr.h>


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//------------------------------------------------------------------------------
namespace zstr
{
	char m_Buf[BUF_LEN_FOR_ITOA];
};


//------------------------------------------------------------------------------
std::string zstr::fmt(const char *_sFormat, ...)
{
	va_list args;
	va_start( args, _sFormat );

	string ret;
	int len = _vscprintf( _sFormat, args ) + 1;
	if(len > 1)
	{
		struct ZSTR_FMT{char *p;int len;ZSTR_FMT():p(0),len(0){}; ~ZSTR_FMT(){if(p)delete p;p=0;len=0;};};
		static ZSTR_FMT buf;
		if(len > buf.len)
		{
			delete buf.p;
			if(len < 128)
				len = 128;
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
std::wstring zstr::wfmt(const wchar_t *_wsFormat, ...)
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
std::wstring zstr::s_to_w(const std::string& s)
{
	int size = (int)s.size() + 1;

	wchar_t *wss = new wchar_t[size];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, wss, size);

	std::wstring ret = wss;
	delete[] wss;
	return ret;

}


//------------------------------------------------------------------------------
std::string zstr::w_to_s(const std::wstring& ws)
{
	int size = (int)ws.size() + 1;

	char *ss = new char[size];
	WideCharToMultiByte(CP_ACP, 0, ws.c_str(), -1, ss, size, NULL, NULL);

	std::string ret = ss;
	delete[] ss;
	return ret;
}


//------------------------------------------------------------------------------
std::string zstr::utf8_to_ansi(const std::string& s)
{
	//переводим в unicode
	int wsize = 2*(int)s.size() + 1;				//длина не более
	wchar_t *wss = new wchar_t[wsize];
	MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, wss, wsize);

	//
	char *ss = new char[wsize];
	WideCharToMultiByte(CP_ACP, 0, wss, -1, ss, wsize, NULL, NULL);

	std::string ret = ss;
	delete[] ss;
	delete[] wss;
	return ret;
}


//------------------------------------------------------------------------------
std::string zstr::ansi_to_utf8(const std::string& s)
{
	//переводим в unicode
	int wsize = 2*(int)s.size() + 1;				//длина не более
	wchar_t *wss = new wchar_t[wsize];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, wss, wsize);

	//
	char *ss = new char[wsize];
	WideCharToMultiByte(CP_UTF8, 0, wss, -1, ss, wsize, NULL, NULL);

	std::string ret = ss;
	delete[] ss;
	delete[] wss;
	return ret;
}


//------------------------------------------------------------------------------
void zstr::ToUpper(string &str)
{
	std::transform(str.begin(), str.end(), str.begin(), toupper);
}


//------------------------------------------------------------------------------
std::string zstr::ToUpper_(const string &str)
{
	string buf = str;
	ToUpper(buf);
	return buf;
}


//------------------------------------------------------------------------------
void zstr::ToUpper(wstring &str)
{
	std::transform(str.begin(), str.end(), str.begin(), toupper);
}


//------------------------------------------------------------------------------
std::wstring zstr::ToUpper_(const wstring &str)
{
	wstring buf = str;
	ToUpper(buf);
	return buf;
}


//------------------------------------------------------------------------------
bool zstr::HaveSubstrNoCase_slow(const char *_str, const char *_substr)
{
	string str(_str);
	string substr(_substr);
	ToUpper(str);
	ToUpper(substr);
	//
	return (strstr(str.c_str(), substr.c_str()) != 0);
}


//------------------------------------------------------------------------------
bool zstr::CompareNoCase(const char *str, const char *str2)
{
	return (_stricmp(str, str2) == 0);
}


//------------------------------------------------------------------------------
bool zstr::is_first(const char *sub_str, const char *full_str)
{
	return !strncmp(sub_str, full_str, strlen(sub_str));
}


//------------------------------------------------------------------------------
bool zstr::is_last(const char *sub_str, const char *full_str)
{
	int len1 = strlen(sub_str);
	int len2 = strlen(full_str);
	int dd = len2 - len1;
	if(dd < 0)
		return false;

	return strncmp(sub_str, full_str + (len2-len1), len1) == 0;
}


//------------------------------------------------------------------------------
bool zstr::is_last(const string &sub_str, const string &full_str)
{
	return is_last(sub_str.c_str(), full_str.c_str());
}



//------------------------------------------------------------------------------
/*std::string zstr::ConvertPath(const string &str)
{
	string ret(str);
	size_t i;
	while( (i = ret.find_first_of('\\')) != string::npos)
		ret[i] = '/';
	//
	return ret;
}*/


static const char *_s_str_for_trim = " \t\n\r\xA0";
//------------------------------------------------------------------------------
void zstr::trim(string& str)
{
	string::size_type pos = str.find_last_not_of(_s_str_for_trim);
	if(pos == string::npos)
		str = string();
	else
	{
		str.erase(pos + 1);
		pos = str.find_first_not_of(_s_str_for_trim);
		if(pos != string::npos)
			str.erase(0, pos);
	}
}


//------------------------------------------------------------------------------
void zstr::trimLast(string& str)
{
	string::size_type pos = str.find_last_not_of(_s_str_for_trim);
	if(pos != string::npos)
		str.erase(pos + 1);
	else
		str = "";
}


//------------------------------------------------------------------------------
std::string zstr::trim_(const string& str)
{
	string ret = str;
	trim(ret);
	return ret;
}


//------------------------------------------------------------------------------
std::string zstr::trimLast_(const string& str)
{
	string ret = str;
	trimLast(ret);
	return ret;
}


//------------------------------------------------------------------------------------------
int zstr::get_first_non_letter_or_num_(const char *str, int nBeg/* = 0*/)
{
	const char *ind = str + nBeg;
	while(char c = *ind)
	{
		if(!textutl::is_letter_eng(c) && !textutl::is_numeric(c))
			return ind - str;

		ind++;
	}
	return -1;
}


//------------------------------------------------------------------------------------------
//то же, что и get_first_non_letter_or_num_, но символы '_' и '-' считает буквами
int zstr::get_first_non_letter_or_num_tag(const char *str, int nBeg/* = 0*/)
{
	const char *ind = str + nBeg;
	while(char c = *ind)
	{
		if(c != '_' && c != '-' && !textutl::is_letter_eng(c) && !textutl::is_numeric(c))
			return ind - str;

		ind++;
	}
	return -1;
}


//------------------------------------------------------------------------------------------
//то же, что и get_first_non_letter_or_num_, но символы '_' и '-' считает буквами
int zstr::get_first_non_letter_or_num_tag_(const zsubstr &substr, int nBeg/* = 0*/)
{
	char c;

	const char *ind = substr.str + nBeg;
	const char *last = substr.str + substr.sz;

	while((c = *ind) != 0 && ind < last)
	{
		if(c != '_' && c != '-' && !textutl::is_letter_eng(c) && !textutl::is_numeric(c))
			return ind - substr.str;

		ind++;
	}
	return -1;
}


//------------------------------------------------------------------------------------------
//add_chars - дополнительные символы, которые тоже пропускаем
//int zstr::get_first_non_letter_or_num_ex(const std::string &str, int nBeg, const std::set<char> &add_chars)
//{
//	for(int ii = nBeg; ii < (int)str.size(); ii++)
//	{
//		char c = str[ii];
//		if(!textutl::is_letter_eng(c) && !textutl::is_numeric(c))
//		{
//			if(add_chars.find(c) == add_chars.end())
//				return ii;
//		}
//	}
//	return -1;
//}


//------------------------------------------------------------------------------------------
//убираем перенос строки с конце
void zstr::remove_eoln(char *str)
{
	int len = (int)strlen(str);
	for(int i = len-1; i >= 0; i--)
	{
		if(str[i] == '\n' || str[i] == '\r')
		{
			str[i] = 0;
			break;
		}
	}
}


//------------------------------------------------------------------------------------------
//производит в строке str замену подстроки substr на new_substr
//производит только одну замену
void zstr::replace_once(std::string &str, const char *_substr, const char *_new_substr)
{
	if(!_substr || !_new_substr)
		return (rel_assert(0));

	size_t jj = str.find(_substr);
	if(jj == string::npos)
		return;

	int substr_len = (int)strlen(_substr);
	str.replace(jj, substr_len, _new_substr);
}


//------------------------------------------------------------------------------------------
//производит в строке str замену подстроки substr на new_substr
void zstr::replace(std::string &str, const char *_substr, const char *_new_substr)
{
	if(!_substr || !_new_substr)
		return (rel_assert(0));

	int substr_len = (int)strlen(_substr);
	for(;;)
	{
		size_t jj = str.find(_substr);
		if(jj == string::npos)
			return;

		str.replace(jj, substr_len, _new_substr);
	}
}


//------------------------------------------------------------------------------------------
//убирает последний '/'
void zstr::remove_last_slash(std::string &str)
{
	//если есть в конце '/' - убираем
	int len = (int)str.length();

	if(!len)
		return;

	if(str[len - 1] == '/')
		str.resize(len - 1);
}


//------------------------------------------------------------------------------------------
//примерно то же, что и std::string::find()
int zstr::find_str(const char * str1, const char * str2)
{
	const char *pch = strstr(str1, str2);
	return pch ? pch - str1 : -1;
}


//------------------------------------------------------------------------------------------
//примерно то же, что и std::string::find()
int zstr::find_str(const char * str1, const char * str2, int nBeg)
{
	const char *pch = strstr(str1 + nBeg, str2);
	return pch ? pch - str1 : -1;
}


//------------------------------------------------------------------------------------------
//функция не тестировалась. взята с http://stackoverflow.com/questions/1031645/how-to-detect-utf-8-in-plain-c
bool zstr::is_utf8(const char * string)
{
    if(!string)
        return 0;

    const unsigned char * bytes = (const unsigned char *)string;
    while(*bytes)
    {
        if( (// ASCII
             // use bytes[0] <= 0x7F to allow ASCII control characters
                bytes[0] == 0x09 ||
                bytes[0] == 0x0A ||
                bytes[0] == 0x0D ||
                (0x20 <= bytes[0] && bytes[0] <= 0x7E)
            )
        ) {
            bytes += 1;
            continue;
        }

        if( (// non-overlong 2-byte
                (0xC2 <= bytes[0] && bytes[0] <= 0xDF) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF)
            )
        ) {
            bytes += 2;
            continue;
        }

        if( (// excluding overlongs
                bytes[0] == 0xE0 &&
                (0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            ) ||
            (// straight 3-byte
                ((0xE1 <= bytes[0] && bytes[0] <= 0xEC) ||
                    bytes[0] == 0xEE ||
                    bytes[0] == 0xEF) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            ) ||
            (// excluding surrogates
                bytes[0] == 0xED &&
                (0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            )
        ) {
            bytes += 3;
            continue;
        }

        if( (// planes 1-3
                bytes[0] == 0xF0 &&
                (0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            ) ||
            (// planes 4-15
                (0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            ) ||
            (// plane 16
                bytes[0] == 0xF4 &&
                (0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            )
        ) {
            bytes += 4;
            continue;
        }

        return 0;
    }

    return 1;
}
