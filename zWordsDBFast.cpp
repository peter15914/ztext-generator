#include "stdafx.h"

#include "zWordsDBFast.h"


//------------------------------------------------------------------------------------------
zWordsDBFast::zWordsDBFast() :
	m_size(0),
	m_strings(0),
	m_offsets(0),
	m_total_size(0)
{
}


//------------------------------------------------------------------------------------------
zWordsDBFast::~zWordsDBFast()
{
	_clear_all();
}


//------------------------------------------------------------------------------------------
void zWordsDBFast::_clear_all()
{
	if(m_strings)
		free(m_strings);
	m_strings = 0;

	if(m_offsets)
		free(m_offsets);
	m_offsets = 0;

	m_size = 0;
	m_total_size = 0;
}


//------------------------------------------------------------------------------------------
bool zWordsDBFast::load_from_file_bin(const std::wstring &file_name)
{
	_clear_all();

	FILE *pFile = _wfopen(file_name.c_str(), L"rb");
	if(!pFile)
		return (rel_assert(0), false);

	//читаем количество слов
	int buf_read = fread(&m_size, sizeof(int), 1, pFile);
	rel_assert(buf_read == 1);

	//читаем размер строкового буфера
	buf_read = fread(&m_total_size, sizeof(int), 1, pFile);
	rel_assert(buf_read == 1);

	//читаем массив строк
	m_strings = (char*)malloc(m_total_size * sizeof(char));
	buf_read = fread(m_strings, sizeof(char), m_total_size, pFile);
	rel_assert(buf_read == m_total_size);

	///читаем массив offsets
	m_offsets = (int*)malloc(m_size * sizeof(int));
	buf_read = fread(m_offsets, sizeof(int), m_size, pFile);
	rel_assert(buf_read == m_size);

	fclose(pFile);
	return true;
}


//------------------------------------------------------------------------------------------
const char *zWordsDBFast::get_str_by_id(int id)
{
	return m_strings + m_offsets[id];
}


//------------------------------------------------------------------------------------------
bool zWordsDBFast::is_valid_id(int id)
{
	return (id >= 0) && (id < m_size);
}


//------------------------------------------------------------------------------------------
bool zWordsDBFast::is_one_letter_word(int id)
{
	const char *s = get_str_by_id(id);
	return (s[0] == 0 || s[1] == 0);
}