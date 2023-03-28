#include "stdafx.h"

#include "zWordPair.h"


//------------------------------------------------------------------------------------------
zWordPair::zWordPair() :
	m_word1(INT_MAX),
	m_word2(INT_MAX)
{
	m_first_cnt = 0;
}


//------------------------------------------------------------------------------------------
zWordPair::~zWordPair()
{
}


//------------------------------------------------------------------------------------------
//add word3 (or increase it's count)
//return current word3 frequency
int zWordPair::add_word3(int word3)
{
	zWords3Map::iterator it = m_Words3Map.find(word3);
	if(it != m_Words3Map.end())
	{
		it->second++;
		return it->second;
	}

	m_Words3Map.insert(std::make_pair(word3, 1));
	return 1;
}


//------------------------------------------------------------------------------------------
//возвращает сколько раз эта пара встречалась (для отладки)
//если будешь использовать эту функцию в реале, то сделай кеширование значения
int zWordPair::get_cnt_4_test()
{
	int ret = 0;

	for(zWords3Map::iterator i = m_Words3Map.begin(); i != m_Words3Map.end(); i++)
		ret += i->second;

	return ret;
}

