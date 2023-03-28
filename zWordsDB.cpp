#include "stdafx.h"

#include "zWordsDB.h"


//------------------------------------------------------------------------------------------
zWordsDB::zWordsDB()
{
	m_Data.reserve(4000000);
}


//------------------------------------------------------------------------------------------
zWordsDB::~zWordsDB()
{
}


//------------------------------------------------------------------------------------------
int zWordsDB::get_word_id(const char *word)
{
	zSrchMap::iterator it = m_SrchMap.find(word);

	if(it == m_SrchMap.end())
		return INT_MAX;
	else
		return it->second;
}


//------------------------------------------------------------------------------------------
zWord &zWordsDB::get_word_by_id(int id)
{
	return m_Data[id];
}


//------------------------------------------------------------------------------------------
const char *zWordsDB::get_str_by_id(int id)
{
	return get_word_by_id(id).m_str.c_str();
}


//------------------------------------------------------------------------------------------
int zWordsDB::add_word(const std::string &word)
{
	int id = get_word_id(word.c_str());
	if(id != INT_MAX)
	{
		zWord &word = get_word_by_id(id);
		word.m_Frequency++;
		return id;
	}

	return _add_new_word(word);
}


//------------------------------------------------------------------------------------------
//операция, обратная add_word (но только уменьшение частоты, удаления из базы не происходит)
void zWordsDB::dec_word(const std::string &word)
{
	int id = get_word_id(word.c_str());
	if(id != INT_MAX)
	{
		zWord &word = get_word_by_id(id);
		word.m_Frequency--;
	}
}


//------------------------------------------------------------------------------------------
int zWordsDB::_add_new_word(const std::string &word)
{
	zWord zword;
	zword.m_str = word;
	zword.m_Frequency++;
	m_Data.push_back(zword);

	int id = m_Data.size() - 1;

	m_SrchMap[m_Data[id].m_str.c_str()] = id;

	return id;
}


//------------------------------------------------------------------------------------------
bool zWordsDB::load_from_file(const std::wstring &file_name)
{
	rel_assert(m_Data.empty() && m_SrchMap.empty());
	std::string s;

	//
	std::ifstream file(file_name.c_str());
	if(!file)
		return (rel_assert(0), false);

	//
	int cnt = 0;
	file >> cnt;
	std::getline(file, s);
	m_Data.reserve(cnt);

	int curFreq = 0;
	while(file)
	{
		std::getline(file, s);
		if(!s.empty())
		{
			if(s[0] == '#')
			{
				curFreq = atoi(s.c_str()+1);
				//dbg_assert(curFreq);
			}
			else
			{
				//dbg_assert(curFreq);

				zWord word;
				word.m_Frequency = curFreq;
				word.m_str = s;
				m_Data.push_back(word);
			}
		}
	}

	_refill_search_map();

	file.close();
	return true;
}


//------------------------------------------------------------------------------------------
bool zWordsDB::save_to_file(const std::wstring &file_name)
{
	zutl::MakeBackupsEx(file_name, 50, L"_bak", 5);
	std::ofstream file(file_name.c_str());
	if(!file)
		return (rel_assert(0), false);

	std::vector<zWord> vec = m_Data;
	//больше не сортируем чтоб не испортить id-шники
	//std::sort(vec.begin(), vec.end(), zwordsdb::cmp_words::cmp);

	//cnt
	file << vec.size() << "\n";

	//words
	int prevFreq = INT_MAX;
	for(size_t i = 0; i < vec.size(); i++)
	{
		if(vec[i].m_Frequency != prevFreq)
		{
			prevFreq = vec[i].m_Frequency;
			file << "#" << zstr::fmt("%07d", prevFreq) << "\n";
		}
		file << vec[i].m_str << "\n";
	}

	file.close();
	return true;
}


//------------------------------------------------------------------------------------------
bool zWordsDB::save_to_file_books_cnt(const std::wstring &file_name)
{
	zutl::MakeBackupsEx(file_name, 10, L"_bak", 2);

	FILE *pFile = _wfopen(file_name.c_str(), L"wb");
	if(!pFile)
		return (rel_assert(0), false);

	//пишем в файл количество слов
	int size = (int)m_aBooksCnt.size();
	int buf_written = fwrite((void*)(&size), sizeof(int), 1, pFile);
	rel_assert(buf_written == 1);
	
	//теперь запишем массив m_aBooksCnt
	fwrite(&m_aBooksCnt[0], sizeof(m_aBooksCnt[0]), size, pFile);

	fclose(pFile);
	return true;
}


//------------------------------------------------------------------------------------------
bool zWordsDB::load_from_file_books_cnt(const std::wstring &file_name)
{
	m_aBooksCnt.clear();

	FILE *pFile = _wfopen(file_name.c_str(), L"rb");
	if(!pFile)
		return (rel_assert(0), false);

	//читаем количество слов
	int size = 0;
	int buf_read = fread(&size, sizeof(int), 1, pFile);
	rel_assert(buf_read == 1);

	m_aBooksCnt.resize(size);
	buf_read = fread(&m_aBooksCnt[0], sizeof(m_aBooksCnt[0]), size, pFile);
	rel_assert(buf_read == size);
	
	fclose(pFile);
	return true;
}


//------------------------------------------------------------------------------------------
//сохраняет в некий бинарный формат, который не хранит всю информацию
//редкоиспользуемая функция, поэтому не слишком оптимальна
bool zWordsDB::save_to_file_bin(const std::wstring &file_name)
{
	FILE *pFile = _wfopen(file_name.c_str(), L"wb");
	if(!pFile)
		return (rel_assert(0), false);

	//пишем в файл количество слов
	int size = (int)m_Data.size();
	int buf_written = fwrite((void*)(&size), sizeof(int), 1, pFile);
	rel_assert(buf_written == 1);

	//подсчитываем суммарный размер строк
	int total_size = 0;
	for(int i = 0; i < size; i++)
		total_size += m_Data[i].m_str.length() + 1;

	//пишем в файл total_size
	buf_written = fwrite((void*)(&total_size), sizeof(int), 1, pFile);
	rel_assert(buf_written == 1);

	//массив, в который будем собирать смещения строк относительно начала массива
	std::vector<int> offsets(size);

	//запишем в файл все слова
	int cur_offset = 0;
	for(int i = 0; i < size; i++)
	{
		std::string &str = m_Data[i].m_str;
		int len = str.length() + 1;
		fwrite(str.c_str(), 1, len, pFile);

		offsets[i] = cur_offset;
		cur_offset += (len);
	}
	rel_assert(total_size == cur_offset);

	//теперь запишем массив offsets
	fwrite(&offsets[0], sizeof(offsets[0]), size, pFile);

	fclose(pFile);
	return true;
}


//------------------------------------------------------------------------------------------
void zWordsDB::_refill_search_map()
{
	rel_assert(m_SrchMap.empty());

	for(size_t i = 0; i < m_Data.size(); i++)
		m_SrchMap[m_Data[i].m_str.c_str()] = i;
}


//------------------------------------------------------------------------------------------
//побегает по всем словам и выдает количество тех, у которых частота <= 0
//и если редкое
int zWordsDB::get_bad_or_rare_words_count()
{
	if(m_aBooksCnt.empty())
		return (rel_assert(0), 0);

	int ret = 0;

	for(size_t i = 0; i < m_Data.size(); i++)
	{
		if(is_rare_or_bad_word(i))
			ret++;
	}

	return ret;
}


//------------------------------------------------------------------------------------------
//возвращает true, если слово редкое или плохое
bool zWordsDB::is_rare_or_bad_word(int id)
{
	if(m_Data[id].m_Frequency <= 10)
		return true;

	if(m_aBooksCnt.empty())
		return (rel_assert(0), false);

	if(m_aBooksCnt[id] <= 10)
		return true;

	return false;
}

