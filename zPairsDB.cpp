#include "stdafx.h"

#include "zPairsDB.h"


//------------------------------------------------------------------------------------------
zPairsDB::zPairsDB()
{
	m_Data.reserve(10000000);
}


//------------------------------------------------------------------------------------------
zPairsDB::~zPairsDB()
{
	_clear_data();
}


//------------------------------------------------------------------------------------------
void zPairsDB::add_tripple(int word1, int word2, int word3, bool is_first)
{
	zWordPair *p = 0;

	//находим, либо создаем zWordPair
	int id = get_pair_id(word1, word2);
	if(id != INT_MAX)
		p = m_Data[id];
	else
	{
		p = new zWordPair();
		p->set_words(word1, word2);
		m_Data.push_back(p);
		m_PairsMap.insert(std::make_pair(p, (int)m_Data.size() - 1));
	}

	//добавляем word3 и если надо - увеличиваем first_cnt
	p->add_word3(word3);
	if(is_first)
		p->inc_first_cnt();
}


//------------------------------------------------------------------------------------------
//return INT_MAX if not exist
int zPairsDB::get_pair_id(int word1, int word2)
{
	static zWordPair buf;
	buf.set_words(word1, word2);

	zPairsMap::iterator it = m_PairsMap.find(&buf);

	if(it == m_PairsMap.end())
		return INT_MAX;
	else
		return it->second;
}


//------------------------------------------------------------------------------------------
/*zWordPair *zPairsDB::get_pair_by_id(int word1, int word2)
{
	int id = get_pair_id(word1, word2);
	return m_Data[id];
}*/


//------------------------------------------------------------------------------------------
bool zPairsDB::load_from_file(const std::wstring &file_name)
{
	return true;
}


//------------------------------------------------------------------------------------------
bool zPairsDB::save_to_file(const std::wstring &file_name)
{
	return true;
}


//------------------------------------------------------------------------------------------
int zPairsDB::get_rand_word(int word1, int word2)
{
	rel_assert(0);	//todo
	return INT_MAX;
}


//------------------------------------------------------------------------------------------
void zPairsDB::_clear_data()
{
	for(size_t i = 0; i < m_Data.size(); i++)
	{
		delete m_Data[i];
	}
}


//------------------------------------------------------------------------------------------
//статистика для отладки
std::string zPairsDB::get_stat_str()
{
	std::string ret;

	ret += zstr::fmt("size = %d. ", (int)m_Data.size());

	int max = 0;
	int total = 0;
	int total_w3maps = 0;

	for(zPairsMap::iterator i = m_PairsMap.begin(); i != m_PairsMap.end(); i++)
	{
		zWordPair *p = i->first;
		int n = p->get_cnt_4_test();
		total += n;
		if(n > max)
			max = n;

		total_w3maps += p->get_word3_map_size_4_test();
	}

	ret += zstr::fmt("max = %d. ", max);
	ret += zstr::fmt("total = %d. ", total);
	ret += zstr::fmt("total_w3maps = %d. ", total_w3maps);

	return ret;
}

