#include "stdafx.h"

#include "zGenProcs.h"

#include "zRWArray.h"

#include <zstlib/zutl.h>
#include <zstlib/textutl.h>

using namespace std;

//если новый режим генерации, то сначала собирается cache
#define NEW_GEN_TYPE_WITH_CACHE


enum
{
	WORDS_IN_CASHE = 3,	//реально не всегда используется, для быстроты
	FIRST_PAIRS_TO_GEN = 35000,
	W3DB_FREAD_BUF_SIZE = (1 << 17)
};


//------------------------------------------------------------------------------------------
inline static __int64 get_pair_from_words(int word1, int word2)
{
	__int64 buf = word1;
	buf = buf << 32;
	buf += word2;
	return buf;
}


//------------------------------------------------------------------------------------------
inline static void get_words_from_pair(__int64 pair_val, int &word1, int &word2)
{
	__int64 w1_64 = pair_val >> 32;
	__int64 w2_64 = pair_val - (w1_64 << 32);

	word1 = (int)w1_64;
	word2 = (int)w2_64;
}


//------------------------------------------------------------------------------------------
//записывает 1 int в 3 байта начиная с arr
inline static void _set_word_to_cache(unsigned char *arr, const int &word)
{
	unsigned char *w =(unsigned char *)((void*)&word);

	*arr = *w;
	arr++;
	w++;

	*arr = *w;
	arr++;
	w++;

	*arr = *w;

	w++;
	rel_assert(*w == 0);
}


//------------------------------------------------------------------------------------------
//считывает 1 int из 3 байтов начиная с arr
inline static void _get_word_from_cache(unsigned char *arr, int &word)
{
	word = 0;

	unsigned char *w =(unsigned char *)((void*)&word);

	*w = *arr;
	arr++;
	w++;

	*w = *arr;
	arr++;
	w++;

	*w = *arr;
}


//------------------------------------------------------------------------------------------
//записывает 3 int'а в 9 байтов начиная с arr
inline static void set_words_to_cache(unsigned char *arr, const int &word1, const int &word2, const int &word3)
{
	_set_word_to_cache(arr, word1);
	_set_word_to_cache(arr+3, word2);
	_set_word_to_cache(arr+6, word3);
}

//------------------------------------------------------------------------------------------
//считывает 3 int'а из 9 байтов начиная с arr
inline static void get_words_from_cache(unsigned char *arr, int &word1, int &word2, int &word3)
{
	_get_word_from_cache(arr, word1);
	_get_word_from_cache(arr+3, word2);
	_get_word_from_cache(arr+6, word3);
}


//------------------------------------------------------------------------------------------
zGenProcs::zGenProcs() :
	m_pairs_cnt(0),
	m_w3db_size(0),
	m_first_pairs_used(0),
	m_dbg_no_cache_read_cnt(0),
	m_dbg_sent_gen_problem_cnt(0),
	m_dbg_wrod_reget_problem_cnt(0),
	m_w3db_fread_cur_ind_(UINT_MAX),
	m_w3db_fread_ind_last(UINT_MAX),
	m_words_db_size(0),
	m_first_pair_4key(-1)
{
}


//------------------------------------------------------------------------------------------
zGenProcs::~zGenProcs()
{
	for(int i = 0; i < (int)m_w3db_files.size(); i++)
	{
		if(m_w3db_files[i].pFile)
			fclose(m_w3db_files[i].pFile);
		m_w3db_files[i].pFile = 0;
	}
}


//------------------------------------------------------------------------------------------
//загружаем необходимые данные из zWordsDB и zWordsDBFast (не храним эти базы, что больше памяти было)
//если передаем word1_4key и word2_4key, то устанавливаем m_first_pair_4key
void zGenProcs::_load_data_from_words_db(const char *word1_4key/* = 0*/, const char *word2_4key/* = 0*/)
{
	zdebug::log()->Log("begin words_db.load_from_file");
	zWordsDB words_db;
	words_db.load_from_file(L"_DB.txt");
	zdebug::log()->Log("end words_db.load_from_file, " + zdebug::log()->GetLogTimeSpend() + "\n");

	//слова, перед которыми ставят запятую
	const char *comma_words[] = {"а", "но", "однако", "ибо", "который", "которая", "которое", "которые"};
	for(int i = 0; i < _countof(comma_words); i++)
	{
		int word_id = words_db.get_word_id(comma_words[i]);
		m_comma_words.insert(word_id);
	}

	//слова, которые не могут быть последними
	const char *not_last_words[] = {"а", "но", "однако", "ибо", "и"};
	for(int i = 0; i < _countof(not_last_words); i++)
	{
		int word_id = words_db.get_word_id(not_last_words[i]);
		m_not_last_words.insert(word_id);
	}

	//грузим words_db_fast и запоминаем из нее данные
	zdebug::log()->Log("begin words_db_fast.load_from_file");

	zWordsDBFast words_db_fast;
	words_db_fast.load_from_file_bin(L"_DB_words.wdb");

	zdebug::log()->Log("end words_db_fast.load_from_file, " + zdebug::log()->GetLogTimeSpend() + "\n");

	//
	m_words_db_size = words_db_fast.get_size();
	for(int i = 0; i < m_words_db_size; i++)
	{
		if(words_db_fast.is_one_letter_word(i))
			m_one_letter_words.insert(i);
	}

	//обрабатываем word1_4key и word2_4key
	if(word1_4key && word2_4key)
	{
		m_first_pair_4key = -1;

		int w1 = words_db.get_word_id(word1_4key);
		int w2 = words_db.get_word_id(word2_4key);
		if(w1 != INT_MAX && w2 != INT_MAX)
			m_first_pair_4key = get_pair_from_words(w1, w2);
	}

}


//------------------------------------------------------------------------------------------
//загрузить всё, что необходимо (массивы, базы)
//если уже загружено, то ничего не делает
//возвращает false, если не получилось, надо прекращать генерацию
bool zGenProcs::_load_all_needed()
{
	if(m_pairs_cnt)
		return true;

	if(W3DB_FREAD_BUF_SIZE > INT_MAX)
		return (rel_assert(0), false);

	srand((unsigned)time(NULL));

	_load_w3db_files();

	m_w3db_fread_buf_.resize(W3DB_FREAD_BUF_SIZE);

	if(m_comma_words.empty())			//теперь можем заранее вызывать в режиме генерации с ключевиками
		_load_data_from_words_db();

	zRWArray::read_array(m_pairs_vec, L"PAIRS9/best50kk.pairs", 0, INT_MAX);
	zRWArray::read_array(m_w3count_vec, L"PAIRS9/words3/words3.cnt", 0, INT_MAX);

	m_pairs_cnt = (int)m_pairs_vec.size();
	rel_assert((int)m_w3count_vec.size() == m_pairs_cnt);

	//проставляем индексы для записи w3count_vec
	for(int i = 1; i < m_pairs_cnt; i++)
		m_w3count_vec[i] += m_w3count_vec[i-1];
	rel_assert(m_w3count_vec[m_w3count_vec.size()-1] == m_w3db_size);

	//проверяем, что для нашей первой пары есть следующие слова
	if(m_first_pair_4key != -1)
	{
		int ind = _get_pair_ind(m_first_pair_4key);
		rel_assert(ind != INT_MAX);

		if(ind != INT_MAX)
		{
			unsigned int max = m_w3count_vec[ind];
			unsigned int min = (ind > 0) ? m_w3count_vec[ind-1] : 0;
			rel_assert(min < max);
			if(min < max)
				zdebug::log()->Log(zstr::fmt("all ok for m_first_pair_4key: %d", max - min));
		}
		else
			return false;		//нет смысла дальше что-то делать
	}

	//
	if(m_first_pair_4key == -1)
	{
		zdebug::log()->Log("begin _generate_first_pairs");
		_generate_first_pairs();
		zdebug::log()->Log("end _generate_first_pairs, " + zdebug::log()->GetLogTimeSpend() + "\n");
	}


#ifdef NEW_GEN_TYPE_WITH_CACHE
	zdebug::log()->Log("begin _create_w3db_cache");
	_create_w3db_cache();
	zdebug::log()->Log("end _create_w3db_cache, " + zdebug::log()->GetLogTimeSpend() + "\n");
#endif
	show_w3db_usage();
	clear_w3db_usage();

	return true;
}


//------------------------------------------------------------------------------------------
//загрузить данные о w3db-файлах
void zGenProcs::_load_w3db_files()
{
	FILE *file = _wfopen(L"PAIRS9/words3_res3/_list", L"r");
	if(!file)
		return (rel_assert(0));

	int cnt = 0;
	fscanf(file, "%d", &cnt);
	m_w3db_files.resize(cnt);

	m_w3db_size = 0;
	unsigned int cur_shift = 0;

	char cbuf[MAX_PATH];
	for(int i = 0; i < cnt; i++)
	{
		fscanf(file, "%s", &cbuf[0]);
		m_w3db_files[i].fname = cbuf;

		wstring fname = L"PAIRS9/words3_res3/";
		fname += zstr::s_to_w(cbuf);

		FILE *pFile = _wfopen(fname.c_str(), L"rb");
		fseek(pFile, 0 , SEEK_END);

		__int64 buf = _ftelli64(pFile);
		buf /= 3;
		rel_assert(buf < UINT_MAX);
		m_w3db_files[i].m_size = (unsigned int)buf;
		m_w3db_files[i].m_shift = cur_shift;
		cur_shift += m_w3db_files[i].m_size;

		m_w3db_files[i].pFile = pFile;
		m_w3db_size += m_w3db_files[i].m_size;
	}

	rel_assert(m_w3db_size < UINT_MAX);

	fclose(file);
}


//------------------------------------------------------------------------------------------
//вернуть индекс пары в массиве pairs_vec, использует бинарный поиск
//INT_MAX, если отсутствует
int zGenProcs::_get_pair_ind(__int64 pair_val)
{
	vector<__int64>::iterator it = lower_bound(m_pairs_vec.begin(), m_pairs_vec.end(), pair_val);
	if(it == m_pairs_vec.end() || *it != pair_val)
		return INT_MAX;

	return it - m_pairs_vec.begin();
}


//------------------------------------------------------------------------------------------
//конвертит предложение в строку
void zGenProcs::gen_sentence_text(std::string &ret, const std::vector<int> &sent, zWordsDBFast &words_db_fast)
{
	rel_assert(ret.empty());

	bool use_prefix = (m_first_pair_4key != -1) && !m_prefix_4key.empty();
	bool use_postfix = (m_first_pair_4key != -1) && !m_postfix_4key.empty();

	if(use_prefix)
		ret = m_prefix_4key;

	//собираем предложение
	for(int i = 0; i < (int)sent.size(); i++)
	{
		int word_id = sent[i];

		if(words_db_fast.is_valid_id(word_id))
		{
			if(use_postfix && i == 2)	//после первой пары выводим постфикс
			{
				ret += ' ';
				ret += m_postfix_4key;
			}

			const char *word = words_db_fast.get_str_by_id(word_id);
			if(i == 0 && !use_prefix)	//первое слово
			{
				//с большой буквы
				string ss = word;
				if(!ss.empty())
					ss[0] = textutl::get_bigger_rus(ss[0]);
				ret += ss;
			}
			else
			{
				//добавляем пробел и возможно запятую
				//не добавляем запятую в режиме с ключевиками
				if(!use_prefix && m_comma_words.find(word_id) != m_comma_words.end())
					ret += ',';
				ret += ' ';

				ret += word;
			}
		}
		else
			rel_assert(0);
	}

	ret += '.';
}


//------------------------------------------------------------------------------------------
//выдает рандомные предложения в файл
void zGenProcs::gen_sentences(const wchar_t *file_name, int sent_cnt, int sent_len_min, int sent_len_max)
{
	//новая фича: если file_name содержит <rand_part>, то заменяем его рандомной строкой,
	//проверяя, что такого файла еще не существует
	wstring new_file_name = file_name;
	size_t jj = new_file_name.find(L"<rand_part>");
	if(jj != string.npos)
	{
		int len = wcslen(L"<rand_part>");

		wstring rand_part = zstr::s_to_w(zutl::GetDateTimeStrFull());
		new_file_name = new_file_name.substr(0, jj-1) + rand_part + new_file_name.substr(jj+len, new_file_name.length());
		if(zutl::FileExists(new_file_name))
			new_file_name += L"_1";
	}

	if(zutl::FileExists(new_file_name))
	{
		_ass(0);
		zutl::MakeBackups(new_file_name, 5);
	}

	//
	FILE *pFile = _wfopen(new_file_name.c_str(), L"w");
	if(!pFile)
		return (rel_assert(0));

	bool ok = _load_all_needed();
	if(!ok)
		return (rel_assert(0));

	//сначала копим строки в векторе, чтобы не писать постоянно на винт (да и сейчас базы слов выгружены)
	vector<vector<int>> m_Cache;
	m_Cache.resize(sent_cnt);
	for(int i = 0; i < (int)m_Cache.size(); i++)
		m_Cache[i].reserve(sent_len_max);

	//
	int untibug = sent_cnt * 20;
	for(int i = 0; (i < sent_cnt) && (--untibug > 0); )
	{
		if(m_first_pair_4key == -1 && m_first_pairs_used >= (int)m_first_pairs_gen.size())
		{
			rel_assert(0);	//не хватило первых пар
			break;
		}

		_gen_sentence(m_Cache[i], sent_len_min, sent_len_max);
		if(!m_Cache[i].empty())
			i++;

		if(i % 50 == 0)
		{
			zdebug::log()->Log(zstr::fmt("sentences generated: %d from %d", i, sent_cnt));
		}
	}

	zdebug::log()->Log("all int sentences generated, " + zdebug::log()->GetLogTimeSpend() + "\n");

	//чистим теперь уже ненужные вектора
	zdebug::log()->Log(zstr::fmt("free m_w3db_cache_"));
	m_w3db_cache_.swap(std::vector<unsigned char>());
	m_w3db_cache_2.swap(std::vector<unsigned char>());
	zdebug::log()->Log("end of free m_w3db_cache_" + zdebug::log()->GetLogTimeSpend() + "\n");

	//грузим words_db_fast
	zdebug::log()->Log("begin words_db_fast.load_from_file");

	zWordsDBFast words_db_fast;
	words_db_fast.load_from_file_bin(L"_DB_words.wdb");

	zdebug::log()->Log("end words_db_fast.load_from_file, " + zdebug::log()->GetLogTimeSpend() + "\n");

	//выводим в файл виде текста
	for(int i = 0; i < (int)m_Cache.size(); i++)
	{
		string s;
		gen_sentence_text(s, m_Cache[i], words_db_fast);
		rel_assert(!s.empty());
		fprintf(pFile, "%s\n", s.c_str());
	}

	zdebug::log()->Log(zstr::fmt("m_dbg_no_cache_read_cnt = %d", m_dbg_no_cache_read_cnt));
	zdebug::log()->Log(zstr::fmt("m_dbg_sent_gen_problem_cnt = %d", m_dbg_sent_gen_problem_cnt));
	zdebug::log()->Log(zstr::fmt("m_dbg_wrod_reget_problem_cnt = %d", m_dbg_wrod_reget_problem_cnt));
	zdebug::log()->Log(zstr::fmt("W3DB_FREAD_BUF_SIZE = %d", W3DB_FREAD_BUF_SIZE));

	fclose(pFile);
}


//------------------------------------------------------------------------------------------
void zGenProcs::test_gen_sentences()
{
	//for(int i = 0; i < 20; i++)
	//{
	//	string s;
	//	gen_sentence(s, 20, 30);
	//	zdebug::log()->Log(s);
	//}
}


//------------------------------------------------------------------------------------------
//генерирует предложение в массив
void zGenProcs::_gen_sentence(vector<int> &sent, int sent_len_min, int sent_len_max)
{
	rel_assert(sent.empty());

	__int64 pair_val = _get_rand_first_pair_fast();
	if(pair_val == -1)
		return;

	int word1 = 0, word2 = 0;
	get_words_from_pair(pair_val, word1, word2);
	bool one_lett1 = _is_one_letter_word(word1);
	bool one_lett2 = _is_one_letter_word(word2);

	sent.push_back(word1);
	sent.push_back(word2);

	for(int ii = 0; ii < sent_len_max; ii++)
	{
		int word3 = _get_rand_word3(pair_val);
		if(word3 == BI_SENT_END)
			break;

		//отсекаем 3 однобуквенных слова подряд
		bool one_lett3 = _is_one_letter_word(word3);
		if(one_lett1 && one_lett2 && one_lett3)
		{
			int untibug = 20;
			while(--untibug)
			{
				m_dbg_wrod_reget_problem_cnt++;

				word3 = _get_rand_word3(pair_val);
				one_lett3 = _is_one_letter_word(word3);
				if(!one_lett3)
					break;
			}
		}
		//отсекаем 3 одинаковых слова подряд
		if(word1 == word2 && word2 == word3)
		{
			int untibug = 20;
			while(--untibug)
			{
				m_dbg_wrod_reget_problem_cnt++;

				word3 = _get_rand_word3(pair_val);
				if(word2 != word3)
					break;
			}
		}

		//не смогли избежать 3 однобуквенных слова подряд
		//или не смогли избежать 3 одинаковых слова подряд
		if(one_lett1 && one_lett2 && one_lett3 || word1 == word2 && word2 == word3)
		{
			sent.clear();
			m_dbg_sent_gen_problem_cnt++;
			//rel_assert(0);
			return;
		}

		//записываем в предложение
		sent.push_back(word3);

		//если уже достигли минимальной длины, то заканчиваем, если последнее слово нам подходит
		if(ii+1 >= sent_len_min)
		{
			if(m_not_last_words.find(word3) == m_not_last_words.end() && !_is_one_letter_word(word3))
				break;
		}

		word1 = word2;
		word2 = word3;

		one_lett1 = one_lett2;
		one_lett2 = one_lett3;

		pair_val = get_pair_from_words(word1, word2);
	}

	//перенсено сюда из функции, кот. раньше была выше уровнем
	if((int)sent.size() < sent_len_min)
		sent.clear();
}


//------------------------------------------------------------------------------------------
//создать заранее сгенеренные первые пары. читает файл first_pairs.dat, а потом выгружает.
//генерит в m_first_pairs_gen некоторое количество первых пар
void zGenProcs::_generate_first_pairs()
{
	zdebug::log()->Log("_generate_first_pairs()");

	m_first_pairs_gen.clear();
	m_first_pairs_used = 0;
	m_first_pairs_gen.reserve(FIRST_PAIRS_TO_GEN);

	//читаем массив из файла
	std::vector<unsigned int> first_pair_vec;
	zRWArray::read_array(first_pair_vec, L"PAIRS9/first_pairs.dat", 0, INT_MAX);
	if(first_pair_vec.size() != m_pairs_vec.size())
		return (rel_assert(0));

	//
	int wcnt = m_words_db_size;
	if(!wcnt)
		return (rel_assert(0));

	//генерим
	while(1)
	{
		int w1 = rand() % wcnt;
		int w2 = rand() % wcnt;

		__int64 pair_val = get_pair_from_words(w1, w2);
		int pair_ind = _get_pair_ind(pair_val);
		if(pair_ind != INT_MAX)
		{
			if(first_pair_vec[pair_ind])
			{
				m_first_pairs_gen.push_back(pair_val);
				if(m_first_pairs_gen.size() >= FIRST_PAIRS_TO_GEN)
					return;
			}
		}
	}
}


//------------------------------------------------------------------------------------------
//выдает "случайную" первую пару из ранее нагенеренного m_first_pairs_gen
__int64 zGenProcs::_get_rand_first_pair_fast()
{
	if(m_first_pair_4key != -1)
		return m_first_pair_4key;

	if(m_first_pairs_used >= (int)m_first_pairs_gen.size())
	{
		return -1;	//больше не перегенеряем, т.к. все-равно в память не поместится
		//_generate_first_pairs();
	}

	return m_first_pairs_gen[m_first_pairs_used++];
}


//------------------------------------------------------------------------------------------
//выдает случайное word3, выдает BI_SENT_END, если больше нет
//каждый раз обращается к винту (new: теперь есть кеш)
int zGenProcs::_get_rand_word3(__int64 pair)
{
	int ind = _get_pair_ind(pair);
	if(ind == INT_MAX)
		return BI_SENT_END;

#ifdef NEW_GEN_TYPE_WITH_CACHE
//берем из кэша
	int word1, word2, word3, word4;
	unsigned char *arr = &m_w3db_cache_[9 * ind];
	unsigned char *arr2 = &m_w3db_cache_2[3 * ind];
	get_words_from_cache(arr, word1, word2, word3);
	_get_word_from_cache(arr2, word4);

	if(word1 != 0xffffff)
	{
		_set_word_to_cache(arr, 0xffffff);
		return word1;
	}
	if(word2 != 0xffffff)
	{
		_set_word_to_cache(arr+3, 0xffffff);
		return word2;
	}
	if(word3 != 0xffffff)
	{
		_set_word_to_cache(arr+6, 0xffffff);
		return word3;
	}
	if(word4 != 0xffffff)
	{
		_set_word_to_cache(arr2, 0xffffff);
		return word4;
	}

	m_dbg_no_cache_read_cnt++;
#endif

	unsigned int max = m_w3count_vec[ind];
	unsigned int min = (ind > 0) ? m_w3count_vec[ind-1] : 0;

	if(max == min)
		return BI_SENT_END;

#ifndef NEW_GEN_TYPE_WITH_CACHE
/// в обычной версии просто возвращаем слово
	//индекс из "массива" 3-х слов
	unsigned int w3_ind = min + rand() % (max-min);
	return _get_word3(w3_ind);
#else
/// в версии с кэшем возвращаем слово и генерим новые два
	unsigned int ind1 = min + rand() % (max-min);
	unsigned int ind2 = min + rand() % (max-min);
	unsigned int ind3 = min + rand() % (max-min);
	unsigned int ind4 = min + rand() % (max-min);
	unsigned int ind5 = min + rand() % (max-min);

	//сортируем
	if(ind1 > ind2) swap(ind1, ind2);
	if(ind2 > ind3) swap(ind2, ind3);
	if(ind3 > ind4) swap(ind3, ind4);
	if(ind4 > ind5) swap(ind4, ind5);
	if(ind1 > ind2) swap(ind1, ind2);
	if(ind2 > ind3) swap(ind2, ind3);
	if(ind3 > ind4) swap(ind3, ind4);
	if(ind1 > ind2) swap(ind1, ind2);
	if(ind2 > ind3) swap(ind2, ind3);
	if(ind1 > ind2) swap(ind1, ind2);

	//
	int ret2 = _get_word3_ex(ind1);

	int w2 = _get_word3_ex(ind2);
	int w3 = _get_word3_ex(ind3);
	int w4 = _get_word3_ex(ind4);
	int w5 = _get_word3_ex(ind5);

	set_words_to_cache(arr, w2, w3, w4);
	_set_word_to_cache(arr2, w5);

	return ret2;
#endif
}


//------------------------------------------------------------------------------------------
//функция устарела, не используем больше
/*
int zGenProcs::_get_word3(unsigned int w3_ind)
{
	if(w3_ind < 0 || w3_ind >= m_w3db_size)
		return (rel_assert(0), BI_SENT_END);

	unsigned int cur_size = 0;
	for(int i = 0; i < (int)m_w3db_files.size(); i++)
	{
		Z_W3DB_FILE &b = m_w3db_files[i];
		if(w3_ind < cur_size + b.m_size)
		{
			//b.m_DbgUsage++;

			unsigned int cur_ind = w3_ind - cur_size;
			rel_assert(cur_ind >= 0 && cur_ind < b.m_size);
			_fseeki64(b.pFile, cur_ind * sizeof(int) , SEEK_SET);

			int ret = BI_SENT_END;
			int bb = fread(&ret, sizeof(int), 1, b.pFile);
			return ret;
		}

		cur_size += b.m_size;
	}

	return (rel_assert(0), BI_SENT_END);
}
*/


//------------------------------------------------------------------------------------------
//для случая перегенерации кэша
int zGenProcs::_get_word3_ex(unsigned int w3_ind)
{
	unsigned int cur_size = 0;
	for(int i = 0; i < (int)m_w3db_files.size(); i++)
	{
		Z_W3DB_FILE &b = m_w3db_files[i];
		if(w3_ind < cur_size + b.m_size)
			return _get_word3_ex(&b, w3_ind);

		cur_size += b.m_size;
	}

	return (rel_assert(0), BI_SENT_END);
}


//------------------------------------------------------------------------------------------
//достает word3 из огроменного файломассива
int zGenProcs::_get_word3_ex(Z_W3DB_FILE *b, unsigned int w3_ind)
{
	if(w3_ind < b->m_shift || w3_ind - b->m_shift >= b->m_size)
		return (rel_assert(0), BI_SENT_END);

	unsigned int cur_ind = w3_ind - b->m_shift;

	/// старая хорошая версия, но медленная
	//{
	//	_fseeki64(b->pFile, cur_ind * sizeof(int), SEEK_SET);

	//	int ret = BI_SENT_END;
	//	int bb = fread(&ret, sizeof(int), 1, b->pFile);
	//	return ret;
	//}


	if(m_w3db_fread_cur_ind_ != UINT_MAX && w3_ind >= m_w3db_fread_cur_ind_ && w3_ind <= m_w3db_fread_ind_last)
	{
		int ind2 = w3_ind - m_w3db_fread_cur_ind_;
		if(ind2 >= 0)
		{
			//попали в cache, всё хорошо
			int ret;
			_get_word_from_cache(&m_w3db_fread_buf_[ind2 * 3], ret);
			return ret;
		}
	}

	//надо считать еще
	//пользуемся тем, что при загрузке мы идем строго вперёд, так что смело читаем W3DB_FREAD_BUF_SIZE,
	//начиная с текущего

	_fseeki64(b->pFile, cur_ind * 3, SEEK_SET);

	//сколько чисел считываем (вдруг скоро конец файла)
	int to_read = W3DB_FREAD_BUF_SIZE / 3;
	if(cur_ind + to_read > b->m_size)
		to_read = b->m_size - cur_ind;

	//считываем
	int bb = fread(&m_w3db_fread_buf_[0], 3, to_read, b->pFile);
	rel_assert(bb == to_read);

	b->m_DbgUsage++;

	m_w3db_fread_cur_ind_ = w3_ind;
	m_w3db_fread_ind_last = w3_ind + to_read - 1;


	int ret;
	_get_word_from_cache(&m_w3db_fread_buf_[0], ret);
	return ret;
}


//------------------------------------------------------------------------------------------
//создать кэш для снижения количества обращений к винту
void zGenProcs::_create_w3db_cache()
{
	if(m_pairs_vec.empty())
		return rel_assert(0);

	int sz = (int)m_pairs_vec.size();
	m_w3db_cache_.resize(sz * 9);	//WORDS_IN_CASHE
	m_w3db_cache_2.resize(sz * 3);

	//переменные для чтения из файлов
	int cur_file_ind = 0;
	Z_W3DB_FILE *b = 0;
	__int64 read_ints = 0;

	int dbg_bad_pairs = 0;	//у скольки пар пустые списки

	//пробегаем все 50kk пар
	for(int ind = 0; ind < sz; ind++)
	{
		if(!b)
		{
			zdebug::log()->Log(zstr::fmt("_create_w3db_cache: new file %d", cur_file_ind));

			b = &m_w3db_files[cur_file_ind++];
			read_ints = 0;
			_fseeki64(b->pFile, 0, SEEK_SET);
		}

		//сколько чисел нужно считать из файла
		unsigned int max = m_w3count_vec[ind];
		unsigned int min = (ind > 0) ? m_w3count_vec[ind-1] : 0;
		int w_cnt = max - min;
		if(!w_cnt)
		{
			dbg_bad_pairs++;
			continue;
		}

		//читаем из файла
		//int bb = fread(&(buf_arr[0]), sizeof(int), w_cnt, b->pFile);
		//if(bb != w_cnt)
		//	return (rel_assert(0));
		//read_ints += bb;
		//if(read_ints >= b->m_size)
		//{
		//	rel_assert(read_ints == b->m_size);
		//	b = 0;
		//}

		//запоминаем рандомные слова
		//m_w3db_cache[2 * ind] = buf_arr[ rand() % w_cnt ];
		//m_w3db_cache[2 * ind + 1] = buf_arr[ rand() % w_cnt ];

		//test
		int ind1 = min + rand() % (max-min);
		int ind2 = min + rand() % (max-min);
		int ind3 = min + rand() % (max-min);
		int ind4 = min + rand() % (max-min);
		if(ind1 > ind2) swap(ind1, ind2);
		if(ind2 > ind3) swap(ind2, ind3);
		if(ind3 > ind4) swap(ind3, ind4);
		if(ind1 > ind2) swap(ind1, ind2);
		if(ind2 > ind3) swap(ind2, ind3);
		if(ind1 > ind2) swap(ind1, ind2);

		//запоминаем рандомные слова
		int w1 = _get_word3_ex( b, ind1 );
		int w2 = _get_word3_ex( b, ind2 );
		int w3 = _get_word3_ex( b, ind3 );
		int w4 = _get_word3_ex( b, ind4 );
		unsigned char *arr = &(m_w3db_cache_[9 * ind]);
		unsigned char *arr2 = &(m_w3db_cache_2[3 * ind]);
		set_words_to_cache(arr, w1, w2, w3);
		_set_word_to_cache(arr2, w4);

		//test
		int ww1, ww2, ww3, ww4;
		get_words_from_cache(arr, ww1, ww2, ww3);
		_get_word_from_cache(arr2, ww4);
		rel_assert(w1 == ww1 && w2 == ww2 && w3 == ww3 && w4 == ww4);

		read_ints += w_cnt;
		if(read_ints >= b->m_size)
		{
			rel_assert(read_ints == b->m_size);
			b = 0;
		}

		//лог
		if(ind % 100000 == 0)
			zdebug::log()->Log(zstr::fmt("_create_w3db_cache: %d from 50kk", ind));
	}

	if(dbg_bad_pairs)
		zdebug::log()->Log(zstr::fmt("dbg_bad_pairs = %d", dbg_bad_pairs));
}


//------------------------------------------------------------------------------------------
//функция конвертирует w3db-базу в базу, которая хранит 3 байта на каждое третье слово
void zGenProcs::convert_w3db_to_3bytes()
{
	_load_w3db_files();

	for(int i = 0; i < (int)m_w3db_files.size(); i++)
	{
		Z_W3DB_FILE &b = m_w3db_files[i];

		wstring fname_in = L"PAIRS9/words3_res_unused/" + zstr::s_to_w(b.fname);
		wstring fname_out = L"PAIRS9/words3_res3/" + zstr::s_to_w(b.fname) + L"3";

		vector<int> vec;
		zRWArray::read_array(vec, fname_in, 0, INT_MAX);

		if(1)
		{
			//тестируем, что совпадают
			vector<unsigned char> vec2;
			zRWArray::read_array(vec2, fname_out, 0, INT_MAX);

			rel_assert(vec.size() == vec2.size() / 3 && vec2.size() % 3 == 0);

			int sz = vec2.size() / 3;
			for(int ii = 0; ii < sz; ii++)
			{
				int word;
				_get_word_from_cache(&vec2[ii*3], word);
				rel_assert(word == vec[ii]);
			}
		}

		if(0)
		{
			//запись в файл
			vector<unsigned char> vec_out;
			vec_out.resize(vec.size() * 3);

			for(int j = 0; j < (int)vec.size(); j++)
				_set_word_to_cache(&(vec_out[j*3]), vec[j]);

			zRWArray::save_vec_to_file(vec_out, fname_out);

			//
			zdebug::log()->Log(zstr::fmt("convert_w3db_to_3bytes %s completed", b.fname.c_str()));
		}
	}

}


//------------------------------------------------------------------------------------------
//для режима генерации с ключевиками
//возвращает false, если не удалось найти пару для word1 и word2
//word1 и word2 - слова, из которх будет состоять первая пара
//prefix и postfix - строки, которые будут писаться перед и после пары <word1, word2>, они не влияют
//на генерацию, а только на вывод в файл
bool zGenProcs::gen_with_first_pair(std::string word1, std::string word2, std::string prefix, std::string postfix)
{
	_load_data_from_words_db(word1.c_str(), word2.c_str());
	if(m_first_pair_4key == -1)
		return false;

	m_prefix_4key = prefix;
	m_postfix_4key = postfix;

	return true;
}

