#pragma once

#include "iWordsDB.h"


//------------------------------------------------------------------------------------------
//класс, который умеет только загружать. но зато быстро
//умеет только получать слово по индексу
class zWordsDBFast : public iWordsDB, public boost::noncopyable
{
	int m_size;			//количество строк
	int m_total_size;	//размер памяти под строки

	//массив строк
	char *m_strings;

	//массив, в который будем собирать смещения строк относительно начала массива
	int *m_offsets;

public:
	zWordsDBFast();
	virtual ~zWordsDBFast();

	bool load_from_file_bin(const std::wstring &file_name);

	const char *get_str_by_id(int id);

	bool is_valid_id(int id);

	bool is_one_letter_word(int id);

	//для отладки
	//и не только
	int get_size() { return m_size; };

private:
	void _clear_all();
};

