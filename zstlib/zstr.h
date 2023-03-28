#pragma once

/*/----------------------------------------------------------------------------
	@namespace zstr
	@desc
		zstr  - некоторые вспомогательные функции для работы с std::string

	2007Jan03 created. zm
-----------------------------------------------------------------------------*/

#include "ztypes.h"

struct zsubstr;

//------------------------------------------------------------------------------
namespace zstr
{
	using std::string;
	using std::wstring;
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
	string ToUpper_(const string &str);
	void ToUpper(wstring &str);
	wstring ToUpper_(const wstring &str);

	bool HaveSubstrNoCase_slow(const char *str, const char *substr);
	bool CompareNoCase(const char *str, const char *str2);
	//
	bool is_first(const char *sub_str, const char *full_str);

	bool is_last(const char *sub_str, const char *full_str);
	bool is_last(const string &sub_str, const string &full_str);
	//
	//string ConvertPath(const string &str);
	//
	void trim(string& str);
	string trim_(const string& str);
	void trimLast(string& str);
	string trimLast_(const string& str);
	//
	std::wstring s_to_w(const std::string& s);
	std::string w_to_s(const std::wstring& ws);

	std::string utf8_to_ansi(const std::string& s);
	std::string ansi_to_utf8(const std::string& s);

	//функция не тестировалась. взята с http://stackoverflow.com/questions/1031645/how-to-detect-utf-8-in-plain-c
	bool is_utf8(const char * s);

	//nBeg - символ, с кооторого начинается поиск
	//return -1, если нет таких символов
	int get_first_non_letter_or_num_(const char *str, int nBeg = 0);

	//то же, что и get_first_non_letter_or_num, но
	//add_chars - дополнительные символы, которые тоже пропускаем
	//int get_first_non_letter_or_num_ex(const std::string &str, int nBeg, const std::set<char> &add_chars);

	//то же, что и get_first_non_letter_or_num_, но символы '_' и '-' считает буквами
	int get_first_non_letter_or_num_tag(const char *str, int nBeg = 0);
	int get_first_non_letter_or_num_tag_(const zsubstr &substr, int nBeg = 0);

	//примерно то же, что и std::string::find()
	int find_str(const char * str1, const char * str2);
	int find_str(const char * str1, const char * str2, int nBeg);

	//убираем перенос строки с конце
	void remove_eoln(char *str);

	//производит в строке str замену подстроки substr на new_substr
	//производит только одну замену
	void replace_once(std::string &str, const char *_substr, const char *_new_substr);

	//производит в строке str замену подстроки substr на new_substr
	void replace(std::string &str, const char *_substr, const char *_new_substr);

	//убирает последний '/'
	void remove_last_slash(std::string &str);

	__forceinline ZBOOL equal(const char *str1, const char *str2)
	{
		return strcmp(str1, str2) == 0;
	}

	__forceinline ZBOOL is_equal_nc(const char *str1, const char *str2)
	{
		return _stricmp(str1, str2) == 0;
	}

	__forceinline ZBOOL is_equal_nc(const std::string &str1, const std::string &str2)
	{
		return _stricmp(str1.c_str(), str2.c_str()) == 0;
	}

	__forceinline ZBOOL empty(const char *str1)
	{
		return !str1 || !(*str1);
	}

};

