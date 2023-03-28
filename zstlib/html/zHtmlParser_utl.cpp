#include "stdafx.h"

#include "zHtmlParser.h"

#include <zstlib/zsubstr.h>


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;



//------------------------------------------------------------------------------------------
//находит первое вхождение sub_str в str, начиная с nBeg, исключая подстроки, находящиеся в
//константных строках (строки в двойных или одинарных кавычках)
//ТЕПЕРЬ ПЕРЕНОС СТРОКИ НЕ ВЫКЛЮЧАЕТ режим константной строки. //old version: перенос строки выключает режим константной строки, т.к. это косяк в html, строковые константы не могут быть на несколько строк файла
//limits - строка, содержащая пары символов - ограничителей (обычно(по дефолту) два вида кавычек, но иногда и скобки)
int zHtmlParser::find_ignore_const_strs_slow(const string &str, const string &sub_str, int nBeg, string limits)
{
	if(sub_str.empty() || limits.empty())
		return (rel_assert(0), -1);

	int sz = (int)str.size();
	int sz2 = (int)sub_str.size();

	//первый символ в искомой строке
	char csrch = sub_str[0];

	char curs_str = 0;	//одинарная или двойная кавычка, означает какой режим сейчас
	for(int ii = nBeg; ii < sz - (sz2-1); ii++)
	{
		char c = str[ii];

		if(curs_str)
		{
			//if(c == '\n')		//перенос строки выключает режим константной строки
			//	curs_str = 0;

			if(c == curs_str)	//закрывающая кавычка
				curs_str = 0;

			//эта часть - внутри константной строки, пропускаем
			continue;
		}

		for(int kk = 0; kk < (int)limits.size(); kk++)
		{
			char lim_c = limits[kk];
			if(c == lim_c)
			{
				curs_str = limits[kk+1];
				break;
			}
		}

		//началась константная строка, пропускаем
		if(curs_str)
			continue;


		if(c == csrch)
		{
			int jj = 1;
			for(; jj < sz2; jj++)
			{
				if(sub_str[jj] != str[ii + jj])
					break;
			}

			if(jj == sz2)	//все символы равны
				return ii;
		}
	}
	return -1;
}


int zHtmlParser::find_ignore_const_strs_(const char *_str, const char *_sub_str, int _nBeg)
{
	//первый символ в искомой строке
	char csrch = _sub_str[0];

	char curs_str = 0;	//одинарная или двойная кавычка, означает какой режим сейчас

	const char *begin = _str + _nBeg;
	const char *ind = begin;
	while(char c = *ind)
	{
		ind++;

		if(curs_str)
		{
			if(c == curs_str)	//закрывающая кавычка
				curs_str = 0;

			//эта часть - внутри константной строки, пропускаем
			continue;
		}

		if(c == '\'' || c == '\"')	//началась константная строка
		{
			curs_str = c;
			continue;
		}

		//константная строка, пропускаем
		if(curs_str)
			continue;


		if(c == csrch)
		{
			const char *i1 = ind;			//уже было ind++
			const char *i2 = _sub_str + 1;

			char cc1, cc2;
			while((cc1 = *i1) != 0 && (cc2 = *i2) != 0)
			{
				if(cc1 != cc2)
					break;

				i1++;
				i2++;
			}

			if(*i2 == 0)			//пробежали всю подстроку, все символы равны
				return (ind - begin) - 1 + _nBeg;
		}
	}
	return -1;
}


int zHtmlParser::find_ignore_const_strs_(zsubstr _str, const char *_sub_str, int _nBeg)
{
	char prev_c = _str.push_last_zero();

	int ret = zHtmlParser::find_ignore_const_strs_(_str.str, _sub_str, _nBeg);

	_str.pop_prev_char(prev_c);

	return ret;
}

