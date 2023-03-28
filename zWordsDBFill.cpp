#include "stdafx.h"

#include "zWordsDBFill.h"

#include "zWordsDB.h"
#include "zTextParser.h"
#include "zBooksDB.h"


//хранит lower-версии букв
static char g_IsValidChar[256];

//------------------------------------------------------------------------------------------
zWordsDBFill::zWordsDBFill() :
	m_MinSentLen(0),
	m_dec_mode(false)
{
	memset(g_IsValidChar, 0, _countof(g_IsValidChar));
	std::string s = "абвгдеёжзийклмнопрстуфхцчшщъыьэюя";
	std::string s2 = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";

	for(size_t i = 0; i < s.length(); i++)
		g_IsValidChar[(unsigned char)s[i]] = s[i];

	for(size_t i = 0; i < s2.length(); i++)
		g_IsValidChar[(unsigned char)s2[i]] = s[i];
}


//------------------------------------------------------------------------------------------
zWordsDBFill::~zWordsDBFill()
{
}


//------------------------------------------------------------------------------------------
void zWordsDBFill::add_words_from_file(zWordsDB *db, const std::wstring &file_name, bool is_fb2)
{
	size_t prev_db_size = db->get_size();

	zTextParser parser;
	parser.open(file_name, is_fb2);

	std::vector<std::string> words;

	std::string str;
	while(1)
	{
		zTextParser::LEX_TYPE type = parser.get_next_word(str);
		if(type == zTextParser::LT_FILE_END)
		{
			_add_words(db, words);
			break;
		}
		if(type == zTextParser::LT_SENTENCE_END)
		{
			_add_words(db, words);
			words.clear();
		}
		else
		if(type == zTextParser::LT_WORD)
			words.push_back(str);
		else
			(rel_assert(0));
	}

	size_t n = db->get_size() - prev_db_size;
	size_t sz = db->get_size();

	std::string encod = parser.is_utf8() ? "utf8" : zstr::fmt("%d", parser.get_encoding());
	m_stat_str = zstr::fmt("%s, %u new words, db size: %u", encod.c_str(), n, sz);
}


//------------------------------------------------------------------------------------------
void zWordsDBFill::add_book_from_file(zBooksDB *db, zWordsDB *words_db, const std::wstring &file_name, bool is_fb2)
{
	m_bad_str = "";
	bool USE_BAD_STR = true;

	rel_assert(m_MinSentLen);

	int book_id = db->add_book(zstr::w_to_s(file_name));
	zBookInfo *book = db->get_book_by_id(book_id);

	zTextParser parser;
	parser.open(file_name, is_fb2);

	//служебная информация
	int snt_cnt = 0;				//общ. кол-во предложений
	int snt_added = 0;				//кол-во добавленных
	int snt_wrong_words = 0;		//кол-во недобавленных из-за плохих слов
	int snt_wrong_len = 0;		//кол-во недобавленных из-за длины


	//массив слов книги
	std::vector<int> word_ids;
	word_ids.reserve(100);

	bool bWasBadWord = false;	//флаг, что в предложении встретилось слово, отсутствующее в базе
	std::string str, dbg_bad_str;
	while(1)
	{
		zTextParser::LEX_TYPE type = parser.get_next_word(str);
		if(type == zTextParser::LT_SENTENCE_END || type == zTextParser::LT_FILE_END)
		{
			//служебная информация
			if(bWasBadWord)
			{
				snt_wrong_words++;
				snt_cnt++;
			}
			else
			if(word_ids.size() > 0)
			{
				snt_cnt++;
				if(word_ids.size() < m_MinSentLen)
					snt_wrong_len++;
				else
					snt_added++;
			}

			if(!bWasBadWord && word_ids.size() >= m_MinSentLen)
			{
				//добавляем предложение в книгу
				for(size_t i = 0; i < word_ids.size(); i++)
					book->add_word(word_ids[i]);
				book->add_word(BI_SENT_END);
			}
			else
			{
				if(USE_BAD_STR)
				{
					m_bad_str += dbg_bad_str;
					m_bad_str += "\n";
				}
			}

			word_ids.clear();
			if(USE_BAD_STR)
				dbg_bad_str = "";

			bWasBadWord = false;
			if(type == zTextParser::LT_FILE_END)
				break;
		}
		else
		if(type == zTextParser::LT_WORD)
		{
			//if(USE_BAD_STR)
			bool bPrev = bWasBadWord;

			if(!bWasBadWord)
			{
				if(!_to_lower(str))
					bWasBadWord = true;
				else
				{
					int word_id = words_db->get_word_id(str.c_str());
					if(word_id == INT_MAX)
						bWasBadWord = true;
					else
						word_ids.push_back(word_id);
				}
			}

			if(USE_BAD_STR)
			{
				dbg_bad_str += ' ';
				if(bPrev == bWasBadWord)
					dbg_bad_str += str;
				else
				{
					dbg_bad_str += ";!!";
					dbg_bad_str += str;
					dbg_bad_str += "!";
				}
			}

		}
		else
			(rel_assert(0));
	}

	rel_assert(snt_cnt == snt_added + snt_wrong_words + snt_wrong_len);

	//служебная информация
	book->set_snt_stat(snt_added, snt_wrong_words, snt_wrong_len);
	m_stat_str = zstr::fmt("cnt=%d, added=%d, wrong_words=%d, wrong_len=%d", snt_cnt, snt_added, snt_wrong_words, snt_wrong_len);
}


//------------------------------------------------------------------------------------------
//true, если добавлены в базу
//words меняется! (для оптимизации)
bool zWordsDBFill::_add_words(zWordsDB *db, std::vector<std::string> &words)
{
	rel_assert(m_MinSentLen);

	if(words.size() < m_MinSentLen)
		return false;

	for(size_t i = 0; i < words.size(); i++)
	{
		if(!_to_lower(words[i]))
			return false;
	}

	for(size_t i = 0; i < words.size(); i++)
	{
		if(m_dec_mode)
			db->dec_word(words[i]);
		else
			db->add_word(words[i]);
	}

	return true;
}


//------------------------------------------------------------------------------------------
//возвращает true, если легальное слово
bool zWordsDBFill::_to_lower(std::string &str)
{
	size_t len = str.length();

	bool was_ = false;
	for(size_t i = 0; i < len; i++)
	{
		char c0 = str[i];
		char c = g_IsValidChar[(unsigned char)c0];
		if(c == 0)
		{
			if(c0 != '-')
				return false;

			//нелья два '-'
			if(was_)
				return false;
			was_ = true;

			//нелья '-' в начале или конце слова
			if(i == 0 || i == len-1)
				return false;
		}
		else
			str[i] = c;
	}

	return true;
}

