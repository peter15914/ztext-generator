#include "stdafx.h"

#include "zsqlutils.h"


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;


//------------------------------------------------------------------------------------------
//конвертнуть строку для записи в sql-дамп
string zsqlutils::_convert_str_for_sql_statement(const string &s, char bad_quote_char)
{
	bool has_bad_chars = (s.find(bad_quote_char) != string.npos);
	if(!has_bad_chars)
		return s;

	string str1;
	str1 += bad_quote_char;

	string repl_str = "\\";
	repl_str += bad_quote_char;

	string ret = s;
	boost::replace_all(ret, str1, repl_str);

	return ret;
}
