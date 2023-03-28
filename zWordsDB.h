#pragma once

#include "iWordsDB.h"


//------------------------------------------------------------------------------------------
class zWord
{
public:
	std::string m_str;
	int m_Frequency;

	zWord() : m_Frequency(0) {};
	virtual ~zWord() {};
};


//------------------------------------------------------------------------------------------
namespace zwordsdb
{
	struct cmp_str
	{
	   bool operator()(const char *a, const char *b) const
	   {
		  return std::strcmp(a, b) < 0;
	   }
	};

	struct cmp_words
	{
	   static bool cmp(const zWord &a, const zWord &b)
	   {
		   return (a.m_Frequency > b.m_Frequency) || (a.m_Frequency == b.m_Frequency && a.m_str < b.m_str);
	   }
	};
}


typedef std::map<const char*, int, zwordsdb::cmp_str> zSrchMap;
//------------------------------------------------------------------------------------------
class zWordsDB : public iWordsDB, public boost::noncopyable
{
	std::vector<zWord> m_Data;
	zSrchMap m_SrchMap;

	std::vector<int> m_aBooksCnt;

public:
	zWordsDB();
	virtual ~zWordsDB();

	//INT_MAX if not found
	int get_word_id(const char *word);

	zWord &get_word_by_id(int id);
	const char *get_str_by_id(int id);

	//return id
	int add_word(const std::string &word);

	//операция, обратная add_word (но только уменьшение частоты, удаления из базы не происходит)
	void dec_word(const std::string &word);

	bool load_from_file(const std::wstring &file_name);
	bool save_to_file(const std::wstring &file_name);

	//сохранить m_aBooksCnt
	bool save_to_file_books_cnt(const std::wstring &file_name);
	bool load_from_file_books_cnt(const std::wstring &file_name);

	//сохраняет в некий бинарный формат, который не хранит всю информацию
	//редкоиспользуемая функция, поэтому не слишком оптимальна
	bool save_to_file_bin(const std::wstring &file_name);

	//для вывода
	size_t get_size() { return m_Data.size(); }

	zSrchMap &get_SrchMap() { return m_SrchMap; }
	std::vector<zWord> &get_Data() { return m_Data; }

	std::vector<int> &get_books_cnt_arr()
	{
		if(m_aBooksCnt.empty())
			m_aBooksCnt.resize(get_size());
		return m_aBooksCnt;
	}

	bool is_valid_id(int id) { return id >= 0 && id < (int)m_Data.size(); }

	//побегает по всем словам и выдает количество тех, у которых частота <= 0
	//и если редкое
	int get_bad_or_rare_words_count();

	//возвращает true, если слово редкое или плохое
	bool is_rare_or_bad_word(int id);

private:
	//return id
	int _add_new_word(const std::string &word);

	void _refill_search_map();
};

