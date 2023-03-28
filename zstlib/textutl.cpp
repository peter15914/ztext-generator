#include "stdafx.h"

#include "textutl.h"


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const char *s_eng_low = "abcdefghijklmnopqrstuvwxyz";
static const char *s_eng_big = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char *s_rus_low = "абвгдеёжзийклмнопрстуфхцчшщъыьэюя";
static const char *s_rus_big = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
static const char *s_eng_translit = " a b v g d e ezh z i j k l m n o p r s t u f htcchshsh   y   ejuja";


static char g_EngCharLow[256];	//lower-версии eng-букв
static char g_RusCharLow[256];	//lower-версии rus-букв
static char g_EngCharBig[256];	//big-версии eng-букв
static char g_RusCharBig[256];	//big-версии rus-букв
static int g_TranslitInd[256];	//индексы в массиве s_eng_translit
static bool g_IsLetterRus[256];	//является ли буквой
static bool g_IsLetterEng[256];	//является ли буквой


//класс-пустышка для инициализации массивов g_EngCharLow и прочих
//------------------------------------------------------------------------------------------
class dummy
{
public:
	dummy()
	{
		for(int c = 0; c < 256; c++)
		{
			g_EngCharLow[c] = (char)c;
			g_RusCharLow[c] = (char)c;
			g_EngCharBig[c] = (char)c;
			g_RusCharBig[c] = (char)c;
			g_TranslitInd[c] = INT_MAX;
			g_IsLetterRus[c] = 0;
			g_IsLetterEng[c] = 0;
		}

		int len = (int)strlen(s_eng_low);
		rel_assert(len == 26 && len == (int)strlen(s_eng_big));
		for(int i = 0; i < len; i++)
		{
			rel_assert( g_EngCharLow[(unsigned char)s_eng_low[i]] == s_eng_low[i] );
			g_EngCharLow[(unsigned char)s_eng_big[i]] = s_eng_low[i];

			rel_assert( g_EngCharBig[(unsigned char)s_eng_big[i]] == s_eng_big[i] );
			g_EngCharBig[(unsigned char)s_eng_low[i]] = s_eng_big[i];
		}

		int len2 = (int)strlen(s_rus_low);
		rel_assert(len2 == 33 && len2 == (int)strlen(s_rus_big));
		for(int i = 0; i < len2; i++)
		{
			rel_assert( g_RusCharLow[(unsigned char)s_rus_low[i]] == s_rus_low[i] );
			g_RusCharLow[(unsigned char)s_rus_big[i]] = s_rus_low[i];

			rel_assert( g_RusCharBig[(unsigned char)s_rus_big[i]] == s_rus_big[i] );
			g_RusCharBig[(unsigned char)s_rus_low[i]] = s_rus_big[i];
		}

		int len3 = (int)strlen(s_eng_translit);
		rel_assert(len3 == 66);

		for(int i = 0; i < 26; i++)
		{
			g_IsLetterEng[(unsigned char)s_eng_low[i]] = true;
			g_IsLetterEng[(unsigned char)s_eng_big[i]] = true;
		}

		for(int i = 0; i < 33; i++)
		{
			unsigned char cc = (unsigned char)s_rus_low[i];
			g_TranslitInd[cc] = i*2;
			g_IsLetterRus[cc] = true;

			g_IsLetterRus[(unsigned char)s_rus_big[i]] = true;
		}
	};
} _dummy;


//------------------------------------------------------------------------------------------
void textutl::to_lower_eng(std::string &str, size_t beg)
{
	for(size_t i = beg; i < str.length(); i++)
		str[i] = g_EngCharLow[(unsigned char)str[i]];
}

//------------------------------------------------------------------------------------------
void textutl::to_lower_rus(std::string &str, size_t beg)
{
	for(size_t i = beg; i < str.length(); i++)
		str[i] = g_RusCharLow[(unsigned char)str[i]];
}


//------------------------------------------------------------------------------------------
void textutl::to_bigger_eng(std::string &str, size_t beg)
{
	for(size_t i = beg; i < str.length(); i++)
		str[i] = g_EngCharBig[(unsigned char)str[i]];
}


//------------------------------------------------------------------------------------------
void textutl::to_bigger_rus(std::string &str, size_t beg)
{
	for(size_t i = beg; i < str.length(); i++)
		str[i] = g_RusCharBig[(unsigned char)str[i]];
}


//------------------------------------------------------------------------------------------
char textutl::get_lower_eng(char c)
{
	return g_EngCharLow[(unsigned char)c];
}


//------------------------------------------------------------------------------------------
char textutl::get_lower_rus(char c)
{
	return g_RusCharLow[(unsigned char)c];
}


//------------------------------------------------------------------------------------------
char textutl::get_bigger_eng(char c)
{
	return g_EngCharBig[(unsigned char)c];
}


//------------------------------------------------------------------------------------------
char textutl::get_bigger_rus(char c)
{
	return g_RusCharBig[(unsigned char)c];
}


//------------------------------------------------------------------------------------------
void textutl::to_translit(const std::string &str, std::string &ret)
{
	ret.clear();

	int len = (int)str.length();
	bool bPrevUnerline = false;
	for(int i = 0; i < len; i++)
	{
		char c0 = str[i];
		//проверяем, вдруг цифра или англ. буква, тогда добавляем как есть
		if(is_letter_eng(c0) || is_numeric(c0))
		{
			ret += c0;
			bPrevUnerline = false;
			continue;
		}

		//
		char low = get_lower_rus(c0);
		bool isBig = (low != c0);

		int ind = g_TranslitInd[(unsigned char) low];
		if(ind == INT_MAX)
		{
			//нет транслитераии, добавляем '_'
			if(!bPrevUnerline)
				ret += '_';
			bPrevUnerline = true;
		}
		else
		{
			bPrevUnerline = false;
			char cc = s_eng_translit[ind++];
			if(cc != ' ')
			{
				ret += isBig ? get_bigger_eng(cc) : cc;
				isBig = false;
			}

			cc = s_eng_translit[ind];
			if(cc != ' ')
			{
				ret += isBig ? get_bigger_eng(cc) : cc;
				isBig = false;
			}
		}
	}
}


//------------------------------------------------------------------------------------------
std::string textutl::to_translit_copy(const std::string &str)
{
	std::string ret;
	to_translit(str, ret);
	return ret;
}


//------------------------------------------------------------------------------------------
bool textutl::is_letter_rus(char c)
{
	return g_IsLetterRus[(unsigned char)c];
}


//------------------------------------------------------------------------------------------
bool textutl::is_letter_eng(char c)
{
	return g_IsLetterEng[(unsigned char)c];
}


//------------------------------------------------------------------------------------------
std::string textutl::get_rand_str(int len)
{
	using namespace std;

	string ret;
	while((int)ret.length() < len)
	{
		char c = (char)(unsigned char)(rand() % 256);
		if(textutl::is_letter_eng(c))
			ret += textutl::get_lower_eng(c);
	}

	return ret;
}


//------------------------------------------------------------------------------------------
bool textutl::has_rus_letters(const std::string &str)
{
	int len = (int)str.length();
	for(int i = 0; i < len; i++)
	{
		if(is_letter_rus(str[i]))
			return true;
	}

	return false;
}


//------------------------------------------------------------------------------------------
ZBOOL textutl::equal_nc(const char *str1, const char *str2)
{
	for(;;)
	{
		char c1 = *str1;
		char c2 = *str2;

		if(!c1)
			return c2 ? ZFALSE : ZTRUE;

		if(!c2)
			return ZFALSE;

		if(c1 != c2)
		{
			if(is_letter_rus(c1))
			{
				if(get_lower_rus(c1) != get_lower_rus(c2))
					return ZFALSE;
			}
			else
			if(is_letter_eng(c1))
			{
				if(get_lower_eng(c1) != get_lower_eng(c2))
					return ZFALSE;
			}
			else
				return ZFALSE;
		}

		str1++;
		str2++;
	}

	return FALSE;
}


//------------------------------------------------------------------------------------------
void textutl::get_translit(char c, char &ret_c1, char &ret_c2)
{
	int ind = g_TranslitInd[(unsigned char) c];
	if(ind == INT_MAX)
		ret_c1 = ret_c2 = 0;
	else
	{
		ret_c1 = s_eng_translit[ind++];
		ret_c2 = s_eng_translit[ind];
		if(ret_c1 == ' ')
			ret_c1 = 0;
		if(ret_c2 == ' ')
			ret_c2 = 0;
	}
}
