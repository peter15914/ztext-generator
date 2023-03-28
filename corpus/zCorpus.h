#pragma once


#include "zStatWords.h"

typedef std::map<std::string, size_t> zCorpData;

//------------------------------------------------------------------------------------------
class zCorpus
{
	std::vector<zStatWords> m_StatWords;
	zCorpData m_Data;

public:
	zCorpus();
	virtual ~zCorpus();

	//добавить данные из файла в корпус
	void add_DataToCorpus(std::string file_name);

	//для теста нагенерим немного текста
	void test_GenSomeText(std::string file_name);

	//для отладки
	const zCorpData &get_Data() { return m_Data; };

private:
	std::string _get_key(const std::string &s1, const std::string &s2);

	zStatWords &get_WordsVec(const std::string &s1, const std::string &s2);
};

