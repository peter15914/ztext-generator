#pragma once


//------------------------------------------------------------------------------------------
class zStatWords
{
	//<слово, количество>
	std::map<std::string, int> m_CollData;

	int m_count;	//общее кол-во слов

public:

	zStatWords();
	virtual ~zStatWords();

	//добавить слово к коллекции
	void add_word(const std::string &str);

	//
	size_t size() { return m_CollData.size(); };

	const std::string &get_word_temp(size_t ind);
};

