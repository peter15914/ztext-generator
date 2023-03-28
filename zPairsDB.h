#pragma once

#include "zWordPair.h"


typedef std::map<zWordPair*, int, zpairsdb::cmp_pairs> zPairsMap;


//------------------------------------------------------------------------------------------
class zPairsDB
{
	std::vector<zWordPair*> m_Data;
	zPairsMap m_PairsMap;

public:
	zPairsDB();
	virtual ~zPairsDB();

	void add_tripple(int word1, int word2, int word3, bool is_first);

	//return INT_MAX if not exist
	int get_pair_id(int word1, int word2);

	//zWordPair *get_pair_by_id(int word1, int word2);

	bool load_from_file(const std::wstring &file_name);
	bool save_to_file(const std::wstring &file_name);

	int get_rand_word(int word1, int word2);

	//статистика для отладки
	std::string get_stat_str();

private:
	void _clear_data();
};
