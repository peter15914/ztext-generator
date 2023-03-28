#include "stdafx.h"

#include "zGenProcs.h"


using namespace std;


//------------------------------------------------------------------------------------------
//тестово показать статистику первых пар
void zGenProcs::show_first_pairs_stat()
{
	//_load_all_needed();

	//__int64 total_sum = 0;
	//int total_first = 0;
	//int total_and = 0;
	//int total_other = 0;
	//for(size_t i = 0; i < m_first_pair_vec.size(); i++)
	//{
	//	unsigned int val = m_first_pair_vec[i];
	//	total_sum += val;
	//	
	//	if(val)
	//	{
	//		total_first++;

	//		int word1 = 0, word2 = 0;
	//		get_words_from_pair(m_pairs_vec[i], word1, word2);
	//		if(word1 == 0)
	//			total_and++;
	//		else
	//			total_other++;
	//	}
	//}

	//string s = zstr::fmt("size=%d, total_first=%d, total_and=%d, total_other=%d",
	//							(int)m_first_pair_vec.size(), total_first, total_and, total_other);
	//zdebug::log()->Log(s);
	//zdebug::log()->Log(zstr::fmt("total_sum=%I64d", total_sum));
}


//------------------------------------------------------------------------------------------
//выводит в лог частоту используемости каждого w3db-файла
void zGenProcs::show_w3db_usage()
{
	for(int i = 0; i < (int)m_w3db_files.size(); i++)
	{
		Z_W3DB_FILE &b = m_w3db_files[i];

		string msg = zstr::fmt("%02d. %s. usage = %d", i, b.fname.c_str(), b.m_DbgUsage);
		zdebug::log()->Log(msg);
	}
}


//------------------------------------------------------------------------------------------
void zGenProcs::clear_w3db_usage()
{
	for(int i = 0; i < (int)m_w3db_files.size(); i++)
		m_w3db_files[i].m_DbgUsage = 0;
}


//------------------------------------------------------------------------------------------
//тесты скорости некоторых функций
int zGenProcs::test_performance()
{
	_load_all_needed();

	int ret = 0;
	for(int i = 0; i < 200000; i++)
	{
		int xx = 0 + rand() % (int)m_pairs_vec.size();
		__int64 pair_val = m_pairs_vec[xx];

		int ind = _get_pair_ind(pair_val);

		ret = ind;
		if(xx != ind)
			rel_assert(0);
	}

	zdebug::log()->Log("test_performance, " + zdebug::log()->GetLogTimeSpend() + "\n");
	return ret;
}


//------------------------------------------------------------------------------------------
void static _test_set_words_to_cache()
{
	//m_w3db_cache_.resize(100);
	//unsigned char *arr = &m_w3db_cache_[0];
	//set_words_to_cache(arr, 1, 2, 3);
	//int w1, w2, w3;
	//get_words_from_cache(arr, w1, w2, w3);

	//m_w3db_cache_.resize(100);
	//unsigned char *arr = &m_w3db_cache_[0];
	//set_words_to_cache(arr, 0xffffff, 0xfffffe, 0xeffff0);
	//int w1, w2, w3;
	//get_words_from_cache(arr, w1, w2, w3);
}
