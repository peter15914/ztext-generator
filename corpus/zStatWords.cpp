#include "stdafx.h"

#include "zStatWords.h"

using namespace std;


//------------------------------------------------------------------------------------------
zStatWords::zStatWords() : m_count(0)
{
}


//------------------------------------------------------------------------------------------
zStatWords::~zStatWords()
{
}

//------------------------------------------------------------------------------------------
//добавить слово к коллекции
void zStatWords::add_word(const std::string &str)
{
	m_CollData[str]++;
	m_count++;
}


//------------------------------------------------------------------------------------------
const std::string &zStatWords::get_word_temp(size_t ind)
{
	static string buf;

	std::map<std::string, int>::iterator it = m_CollData.begin();
	while(ind-- && it != m_CollData.end())
		it++;

	if(it != m_CollData.end())
		return it->first;

	return buf;
}
