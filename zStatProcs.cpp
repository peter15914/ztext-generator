#include "stdafx.h"

#include "zStatProcs.h"

#include "zBooksDBProcs.h"
#include "zBooksDB.h"
#include "zBooksDBProcs.h"
#include "zWordsDBFast.h"
#include "zRWArray.h"


//------------------------------------------------------------------------------------------
zStatProcs::zStatProcs() :
	m_val_min(0),
	m_val_max(0)
{
}


//------------------------------------------------------------------------------------------
void zStatProcs::create_pairs_stat_file()
{
	std::wstring sDBFileName = L"_DB.txt";
	//std::wstring sDBFileNameBin = L"_DB_words.wdb";
	std::wstring sDBBooksCntFileName = L"_DB_books_cnt.txt";

	zBooksDBProcs dbProcs;
	dbProcs.load_books_list(L"__books_list");

	zWordsDB words_db;
	zdebug::log()->Log("begin load_from_file_bin");
	words_db.load_from_file_books_cnt(sDBBooksCntFileName);
	words_db.load_from_file(sDBFileName);
	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");

	//
	zdebug::log()->ResetTime();
	zdebug::log()->Log("begin");

	std::set<__int64> pairs_set;
	int book_num = 0;
	int added_total = 0;
	int file_num = 0;
	//
	zBooksDB booksDB;
	const std::vector<std::wstring> &files = dbProcs.get_DB_files_list();
	for(size_t i = 26; i < files.size(); i++)
	{
		if(booksDB.begin_load_from_file_bin(files[i]))
		{
			while(booksDB.to_next_book())
			{
				zBookInfo *book = booksDB.get_cur_book();
				if(!dbProcs.is_bad_book(book->get_name()))	//хорошая книга
				{
					const std::vector<int> &words = book->get_words();
					int cnt = (int)words.size();
					if(cnt < 3)
						continue;

					book_num++;

					int w0 = BI_SENT_END;
					int w1 = BI_SENT_END;
					int w2 = words[0];
					int w3 = words[1];

					//пробегаем все слова книги
					for(int ii = 2; ii < cnt; ii++)
					{
						w0 = w1;
						w1 = w2;
						w2 = w3;
						w3 = words[ii];

						if(w1 != BI_SENT_END && w2 != BI_SENT_END)
						{
							if(!words_db.is_rare_or_bad_word(w1) && !words_db.is_rare_or_bad_word(w2))
							{
								__int64 buf = w1;
								buf = buf << 32;
								buf += w2;
								pairs_set.insert(buf);
								added_total++;
							}
						}
					}

					if(book_num % 100 == 0)
					{
						zdebug::log()->LogImp(zstr::fmt("%d book, size: %d, added_total: %d", book_num, (int)pairs_set.size(), added_total));
					}
				}

				//если слишком много эелементов в pairs_set, то сохраняем в файл
				if(pairs_set.size() >= 20000000)
				{
					std::wstring file_name = zstr::wfmt(L"PAIRS/%03d_%03d", i, ++file_num);
					output_pairs_file(pairs_set, file_name);
					pairs_set.clear();
				}
			}//while(booksDB.to_next_book())
		}
		zdebug::log()->Log(zstr::fmt("%d/%d finished\n", (int)i, (int)files.size()));
	}

	//если слишком много эелементов в pairs_set, то сохраняем в файл
	if(!pairs_set.empty())
	{
		std::wstring file_name = zstr::wfmt(L"PAIRS/last_%03d", ++file_num);
		output_pairs_file(pairs_set, file_name);
		pairs_set.clear();
	}

	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");

}


//------------------------------------------------------------------------------------------
bool zStatProcs::output_pairs_file(std::set<__int64> &pairs_set, std::wstring file_name)
{
	FILE *pFile = _wfopen(file_name.c_str(), L"wb");
	if(!pFile)
		return (rel_assert(0), false);

	//пишем в файл размер
	int size = pairs_set.size();
	int buf_written = fwrite((void*)(&size), sizeof(int), 1, pFile);
	rel_assert(buf_written == 1);

	__int64 prev = -1;
	for(std::set<__int64>::iterator it = pairs_set.begin(); it != pairs_set.end(); it++)
	{
		__int64 buf = *it;
		rel_assert(prev == -1 || prev < buf);
		prev = buf;
		//
		int buf_written = fwrite(&buf, sizeof(__int64), 1, pFile);
		rel_assert(buf_written == 1);
	}

	fclose(pFile);

	zdebug::log()->Log(zstr::fmt("pairs file size: %d", size));
	return true;
}


//------------------------------------------------------------------------------------------
//для merge_pairs_files
static int _outp_on_merge(FILE *file_out, __int64 val, __int64 &prev_outp_val)
{
	if(prev_outp_val != val)
	{
		rel_assert(prev_outp_val < val);

		prev_outp_val = val;
		int buf_written = fwrite(&val, sizeof(__int64), 1, file_out);
		rel_assert(buf_written == 1);
		return 1;
	}
	return 0;
}


//------------------------------------------------------------------------------------------
bool zStatProcs::merge_pairs_files(std::wstring file_name_1, std::wstring file_name_2, std::wstring file_name_out)
{
	int bb1 = 0, bb2 = 0;
	int result_size = 0;
	__int64 prev_outp_val = _I64_MIN;

	zdebug::log()->Log("begin merge_pairs_files");
	zdebug::log()->Log(zstr::w_to_s(file_name_1 + L" + " + file_name_2 + L" = " + file_name_out));
	zdebug::log()->ResetTime();

	FILE *file1 = _wfopen(file_name_1.c_str(), L"rb");
	FILE *file2 = _wfopen(file_name_2.c_str(), L"rb");
	FILE *file_out = _wfopen(file_name_out.c_str(), L"wb");
	if(!file1 || !file2 || !file_out)
		return (rel_assert(0), false);

	//размеры файлов
	fseek(file1 , 0 , SEEK_END);
	fseek(file2 , 0 , SEEK_END);
	__int64 fsize1 = _ftelli64(file1);
	__int64 fsize2 = _ftelli64(file2);
	rewind(file1);
	rewind(file2);
	fsize1 /= sizeof(__int64);
	fsize2 /= sizeof(__int64);

	zdebug::log()->Log(zstr::fmt("file1 size: %d", fsize1));
	zdebug::log()->Log(zstr::fmt("file2 size: %d", fsize2));

	////читаем размеры из файлов (сохранял зачем-то) и сравниваем
	//int size1 = 0, size2 = 0;
	//bb1 = fread(&size1, sizeof(int), 1, file1);
	//bb2 = fread(&size2, sizeof(int), 1, file2);
	//rel_assert(bb1 == 1 && bb2 == 1);
	//if(fsize1 != size1 || fsize2 != size2)
	//	return (rel_assert(0), false);

	//мерджим файлы
	__int64 prevVal1, prevVal2;
	bb1 = fread(&prevVal1, sizeof(__int64), 1, file1);
	bb2 = fread(&prevVal2, sizeof(__int64), 1, file2);
	rel_assert(bb1 == 1 && bb2 == 1);

	//кол-во использованных int64
	__int64 used1 = 0;
	__int64 used2 = 0;

	while(1)
	{
		if(used1 >= fsize1 && used2 >= fsize2)
			break;

		bool bUse1 = true;

		if(used1 >= fsize1)	//используем 2-ой файл
			bUse1 = false;
		else
		if(used2 >= fsize2)	//используем 1-ый файл
			bUse1 = true;
		else	//выбираем нужный
			bUse1 = (prevVal1 <= prevVal2);

		if(bUse1)
		{
			result_size += _outp_on_merge(file_out, prevVal1, prev_outp_val);
			bb2 = fread(&prevVal1, sizeof(__int64), 1, file1);
			used1++;
		}
		else
		{
			result_size += _outp_on_merge(file_out, prevVal2, prev_outp_val);
			bb2 = fread(&prevVal2, sizeof(__int64), 1, file2);
			used2++;
		}
	}
	
	fclose(file1);
	fclose(file2);
	fclose(file_out);

	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");
	zdebug::log()->Log(zstr::fmt("result file size: %d", result_size));

	return true;
}


//------------------------------------------------------------------------------------------
//дозагрузить всё, что необходимо
void zStatProcs::_load_all_needed()
{
	if(m_words_db.get_size())
		return;

	std::wstring sDBFileName = L"_DB.txt";
	std::wstring sDBBooksCntFileName = L"_DB_books_cnt.txt";

	zdebug::log()->Log("begin words_db.load_from_file");
	m_words_db.load_from_file_books_cnt(sDBBooksCntFileName);
	m_words_db.load_from_file(sDBFileName);
	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");
}


//------------------------------------------------------------------------------------------
void zStatProcs::test_output_one_letter_sent()
{
	zdebug::log()->Log("test_output_one_letter_sent");

	_load_all_needed();
	_fill_is_one_letter();

	zBooksDB booksDB;
	zBooksDBProcs dbProcs;
	dbProcs.load_books_list(L"__books_list");

	//
	int book_num = 0;
	__int64 added_total_ = 0;
	std::vector<int> cur_sent_words(500);

	const std::vector<std::wstring> &files = dbProcs.get_DB_files_list();
	for(size_t i = 0; i < files.size(); i++)
	//for(size_t i = 0; i < 1; i++)
	{
		bool t = booksDB.begin_load_from_file_bin(files[i]);
		if(!t)
		{
			zdebug::log()->Log(zstr::fmt("%d/%d finished\n", (int)i, (int)files.size()));
			break;
		}
		//
		while(booksDB.to_next_book())
		{
			zBookInfo *book = booksDB.get_cur_book();
			if(dbProcs.is_bad_book(book->get_name()))	//плохая книга
				continue;

			const std::vector<int> &words = book->get_words();
			int cnt = (int)words.size();
			if(cnt < 3)
				continue;

			book_num++;

			bool cur_sent_bad = true;
			//пробегаем все слова книги
			for(int ii = 0; ii < cnt; ii++)
			{
				int w = words[ii];
				if(w == BI_SENT_END)
				{
					if(cur_sent_bad)
					{
						std::string s;
						for(int jj = 0; jj < (int)cur_sent_words.size(); jj++)
						{
							s += ' ';
							s += m_words_db.get_str_by_id(cur_sent_words[jj]);
						}
						zdebug::log()->Log(s);
					}

					cur_sent_bad = true;
					cur_sent_words.clear();
				}
				else
				{
					if(!m_is_one_letter[w])
						cur_sent_bad = false;
					if(cur_sent_bad)
						cur_sent_words.push_back(w);
				}
			}

			if(book_num % 100 == 0)
				zdebug::log()->LogImp(zstr::fmt("%d book, added_total: %I64d", book_num, added_total_));
		}
	}
}


//------------------------------------------------------------------------------------------
void zStatProcs::_fill_is_one_letter()
{
	rel_assert(m_words_db.get_size());

	//находим слова, состоящие из одной буквы
	//zdebug::log()->Log("is_one_letter words:");
	m_is_one_letter.resize(m_words_db.get_size(), false);

	for(int i = 0; i < (int)m_words_db.get_size(); i++)
	{
		if(!m_words_db.is_rare_or_bad_word(i))
		{
			const char *s = m_words_db.get_str_by_id(i);
			rel_assert(s);
			if(s && s[0] != 0 && s[1] == 0)
			{
				m_is_one_letter[i] = true;
				//zdebug::log()->Log(zstr::fmt("%s (%d)", s, m_words_db.get_word_by_id(i).m_Frequency));
			}
		}
	}
}


//------------------------------------------------------------------------------------------
//считаем статистику для пар из диапазона [ind_first, ind_last)
void zStatProcs::check_pairs_stat(__int64 ind_first, __int64 ind_last)
{
	_load_all_needed();
	cnt_vec.clear();
	pairs_vec.clear();
	m_is_one_letter.clear();

	zdebug::log()->Log(zstr::fmt("begin check_pairs_stat %I64d %I64d", ind_first, ind_last));
	zdebug::log()->ResetTime();

	int size = _read_pairs(L"PAIRS8/08_000_051", ind_first, ind_last);
	zdebug::log()->Log(zstr::fmt("size is %d", size));

	cnt_vec.resize(size);

	_fill_is_one_letter();

	zBooksDB booksDB;
	zBooksDBProcs dbProcs;
	dbProcs.load_books_list(L"__books_list");

	//
	int book_num = 0;
	__int64 added_total_ = 0;

	m_val_min = pairs_vec[0];
	m_val_max = pairs_vec[pairs_vec.size()-1];

	const std::vector<std::wstring> &files = dbProcs.get_DB_files_list();
	for(size_t i = 0; i < files.size(); i++)
	//for(size_t i = 0; i < 1; i++)
	{
		bool t = booksDB.begin_load_from_file_bin(files[i]);
		if(!t)
		{
			zdebug::log()->Log(zstr::fmt("%d/%d finished\n", (int)i, (int)files.size()));
			break;
		}
		//
		while(booksDB.to_next_book())
		{
			zBookInfo *book = booksDB.get_cur_book();
			if(dbProcs.is_bad_book(book->get_name()))	//плохая книга
				continue;

			const std::vector<int> &words = book->get_words();
			int cnt = (int)words.size();
			if(cnt < 3)
				continue;

			book_num++;

			bool cur_sent_bad = true;
			int cur_sent_ind = 0;
			//пробегаем все слова книги
			for(int ii = 0; ii < cnt; ii++)
			{
				int w = words[ii];
				if(w == BI_SENT_END)
				{
					if(!cur_sent_bad)
					{
						//хорошие предложения используем для статистики
						for(int jj = cur_sent_ind; jj < ii-1; jj++)
							_add_pairs_cnt(words[jj], words[jj+1]);
						added_total_ += (ii-cur_sent_ind);
					}

					cur_sent_bad = true;
					cur_sent_ind = ii+1;
				}
				else
				{
					if(cur_sent_bad && !m_is_one_letter[w])
						cur_sent_bad = false;
				}
			}

			if(book_num % 500 == 0)
			{
				zdebug::log()->LogImp(zstr::fmt("%d book, added_total: %I64d", book_num, added_total_));
				//break;
			}
		}
	}

	//
	std::wstring fname = zstr::wfmt(L"PAIRS8_stat/%I64d_%I64d.stat", ind_first, ind_last);
	zRWArray::save_vec_to_file(cnt_vec, fname);
}


//------------------------------------------------------------------------------------------
//читает пары в pairs_vec, возвращает размер
int zStatProcs::_read_pairs(std::wstring file_name, __int64 ind_first, __int64 ind_last)
{
	return zRWArray::read_array(pairs_vec, file_name, ind_first, ind_last);
}


//------------------------------------------------------------------------------------------
//вернуть индекс пары в массиве pairs_vec, использует бинарный поиск
//INT_MAX, если отсутствует
int zStatProcs::_get_pair_ind(__int64 pair_val)
{
	std::vector<__int64>::iterator it = std::lower_bound(pairs_vec.begin(), pairs_vec.end(), pair_val);
	if(it == pairs_vec.end() || *it != pair_val)
		return INT_MAX;

	return it - pairs_vec.begin();
}


//------------------------------------------------------------------------------------------
void zStatProcs::test_binary_search()
{
	zdebug::log()->Log("begin check_pairs_stat");
	zdebug::log()->ResetTime();

	int size = _read_pairs(L"PAIRS8/08_000_051", 0, 10000000);
	zdebug::log()->Log(zstr::fmt("size is %d", size));

	for(int i = 0; i < 10000000; i++)
	{
		int ind = rand() % size;
		__int64 val = pairs_vec[ind];

		int ind2 = _get_pair_ind(val);
		rel_assert(ind2 == ind);
	}

	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");
}


//------------------------------------------------------------------------------------------
//конвертнуть бинарный формат cnt_vec в текстовый
//ind_first и ind_last указываются руками
bool zStatProcs::save_to_text_format(std::wstring pairs_file_name, __int64 ind_first, __int64 ind_last,
										std::wstring file_name1, std::wstring file_name2)
{
	FILE *file1 = _wfopen(file_name1.c_str(), L"rb");
	FILE *file2 = _wfopen(file_name2.c_str(), L"w");
	if(!file1 || !file2)
		return (rel_assert(0), false);

	//размер файла
	fseek(file1, 0 , SEEK_END);
	__int64 fsize1 = _ftelli64(file1);
	rewind(file1);
	fsize1 /= sizeof(unsigned int);
	
	//читаем pairs_vec
	int buf_size = _read_pairs(pairs_file_name, ind_first, ind_last);
	if(buf_size != fsize1)
		return (rel_assert(0), false);

	//грузим zWordsDBFast
	zWordsDBFast words_db_fast;
	words_db_fast.load_from_file_bin(L"_DB_words.wdb");

	//читаем файл со статистикой
	unsigned int buf;
	int buf_read;
	for(int i = 0; i < fsize1; i++)
	{
		buf_read = fread(&buf, sizeof(unsigned int), 1, file1);
		rel_assert(buf_read == 1);

		int w1 = 0, w2 = 0;
		get_words_from_pair(pairs_vec[i], w1, w2);

		if(!words_db_fast.is_valid_id(w1) || !words_db_fast.is_valid_id(w2))
		{
			rel_assert(0);
			fprintf(file2, "!!!bad  %d %d %u\n", w1, w2, buf);
		}
		else
		{
			const char *s1 = words_db_fast.get_str_by_id(w1);
			const char *s2 = words_db_fast.get_str_by_id(w2);
			fprintf(file2, "%s %s %u\n", s1, s2, buf);
		}
	}

	fclose(file1);
	fclose(file2);

	zdebug::log()->Log("end save_to_text_format");
	return true;
}


//------------------------------------------------------------------------------------------
//увеличивает количество пар для статистики в cnt_vec
void zStatProcs::_add_pairs_cnt(const int &word1, const int &word2)
{
	if(word1 == BI_SENT_END || word2 == BI_SENT_END)
	{
		rel_assert(0);
		return;
	}

	if(m_words_db.is_rare_or_bad_word(word1) || m_words_db.is_rare_or_bad_word(word2))
		return;

	__int64 buf = get_pair_from_words(word1, word2);

	//вставляем
	if(buf >= m_val_min && buf <= m_val_max)
	{
		int ind = _get_pair_ind(buf);
		if(ind != INT_MAX)
		{
			if(cnt_vec[ind] < UINT_MAX)
			{
				cnt_vec[ind]++;
				if(cnt_vec[ind] == UINT_MAX)
				{
					zdebug::log()->LogImp(zstr::fmt("cnt_vec[ind] == UINT_MAX. <%s, %d> <%s, %d>",
						m_words_db.get_str_by_id(word1), word1, m_words_db.get_str_by_id(word2), word2));
				}
			}
		}
		else
			(rel_assert(0));
	}
}


//------------------------------------------------------------------------------------------
bool zStatProcs::test_find_rare_pairs(std::wstring file_name)
{
	FILE *file1 = _wfopen((L"PAIRS9/" + file_name).c_str(), L"rb");
	//FILE *file1 = _wfopen((L"PAIRS8_stat/" + file_name).c_str(), L"rb");
	if(!file1)
		return (rel_assert(0), false);

	//размер файла
	fseek(file1, 0 , SEEK_END);
	__int64 fsize1 = _ftelli64(file1);
	rewind(file1);
	fsize1 /= sizeof(unsigned int);
	
	size_t test_size = 1001;
	std::vector<unsigned int> cnt_stat(test_size);

	//читаем файл со статистикой
	unsigned int buf;
	int buf_read;
	unsigned int total_sum = 0;
	for(int i = 0; i < fsize1; i++)
	{
		buf_read = fread(&buf, sizeof(unsigned int), 1, file1);
		rel_assert(buf_read == 1);

		if(buf < test_size-1)
			cnt_stat[buf]++;
		else
			cnt_stat[test_size-1]++;

		total_sum += buf;
	}

	std::ofstream file2((L"____aaa.txt" + file_name).c_str());
	file2 << "total sum is: " << total_sum << "\n";
	unsigned int total = 0;
	for(int i = 0; i < (int)test_size; i++)
	{
		total += cnt_stat[i];
		std::string s = zstr::fmt("%03d:%07u, total: %07u", i, cnt_stat[i], total);
		file2 << s << "\n";
	}
	file2.close();

	fclose(file1);
	return true;
}


//------------------------------------------------------------------------------------------
//вывести окончательный, хороший файл с парами
//содержит только пары, встретившиеся чаще 6 раз (чоб было около 50 млн.)
bool zStatProcs::output_best_pairs_file()
{
	zdebug::log()->ResetTime();

	//открываем файл
	FILE *file_pairs = _wfopen(L"PAIRS8/08_000_051", L"rb");
	FILE *file_out_1 = _wfopen(L"PAIRS9/best50kk.pairs", L"wb");
	FILE *file_out_2 = _wfopen(L"PAIRS9/best50kk.stat", L"wb");
	FILE *file_stat = 0;

	__int64 totalWrite = 0;

	if(!file_pairs || !file_out_1 || !file_out_2)
		return (rel_assert(0), 0);

	const char *statFiles[10] = { "0_40000000.stat", "40000000_80000000.stat", "80000000_120000000.stat",
		"120000000_160000000.stat", "160000000_200000000.stat", "200000000_240000000.stat", "240000000_280000000.stat",
		"280000000_320000000.stat", "320000000_370000000.stat", "370000000_430000000.stat" };

	//размер файла
	fseek(file_pairs , 0 , SEEK_END);
	__int64 file_pairs_size = _ftelli64(file_pairs);
	rewind(file_pairs);
	file_pairs_size /= sizeof(__int64);
	zdebug::log()->Log(zstr::fmt("file_pairs size: %u", file_pairs_size));


	//для чтения из текущего файла статистики
	__int64 statFileSize = 0, readStatFile = 0;
	int cur_stat_file = -1;

	//пробегаем по file_pairs и попутно по файлам со статистикой
	//выводим только те пары, у которых частота >= 7
	__int64 buf64 = 0;
	unsigned int ubuf = 0;
	int bb1 = 0, bb2 = 0;
	for(__int64 i = 0; i < file_pairs_size; i++)
	{
		if(readStatFile >= statFileSize)
		{
			//открываем новый файл со статистикой
			if(file_stat)
				fclose(file_stat);
			cur_stat_file++;
			if(cur_stat_file >= 10)
			{
				rel_assert(0);
				break;
			}
			std::string fname = "PAIRS8_stat/";
			fname += statFiles[cur_stat_file];
			file_stat = fopen(fname.c_str(), "rb");
			//размер файла
			fseek(file_stat , 0 , SEEK_END);
			statFileSize = _ftelli64(file_stat);
			rewind(file_stat);
			statFileSize /= sizeof(unsigned int);
			zdebug::log()->Log(zstr::fmt("%s. statFileSize: %u", fname.c_str(), statFileSize));
			readStatFile = 0;
		}

		bb1 = fread(&buf64, sizeof(__int64), 1, file_pairs);
		rel_assert(bb1 == 1);

		bb2 = fread(&ubuf, sizeof(unsigned int), 1, file_stat);
		rel_assert(bb2 == 1);
		readStatFile++;
		
		if(ubuf >= 7)
		{
			bb1 = fwrite((void*)(&buf64), sizeof(__int64), 1, file_out_1);
			bb2 = fwrite((void*)(&ubuf), sizeof(unsigned int), 1, file_out_2);
			rel_assert(bb1 == 1 && bb2 == 1);
			totalWrite++;
		}

		//progress
		if(i % 1000000 == 0)
			zdebug::log()->Log(zstr::fmt("pairs processed: %u, time ", i) + zdebug::log()->GetLogTimeSpend().c_str());
	}

	fclose(file_pairs);
	fclose(file_out_1);
	fclose(file_out_2);
	if(file_stat)
		fclose(file_stat);

	zdebug::log()->Log(zstr::fmt("totalWrite: %u", totalWrite));

	return true;
}


//------------------------------------------------------------------------------------------
//подсчитать, сколько после каждой пары может идти третьих слов
bool zStatProcs::calc_pairs_word3()
{
	zdebug::log()->Log(";---------------------------------");
	_read_pairs(L"PAIRS9/best50kk.pairs", 0, 999999999);
	//_read_array(L"PAIRS9/best50kk.stat", 0, 999999999, false);

	//if(cnt_vec.size() != pairs_vec.size())
	//	return (rel_assert(0), false);

	int size = (int)pairs_vec.size();
	w3count_vec.clear();
	w3count_vec.resize(size);

	zBooksDB booksDB;
	zBooksDBProcs dbProcs;
	dbProcs.load_books_list(L"__books_list");

	_load_all_needed();
	_fill_is_one_letter();

	//
	int book_num = 0;
	__int64 dbg_added = 0, dbg_not_added = 0;

	//m_val_min = pairs_vec[0];
	//m_val_max = pairs_vec[pairs_vec.size()-1];

	const std::vector<std::wstring> &files = dbProcs.get_DB_files_list();
	for(size_t i = 0; i < files.size(); i++)
	//for(size_t i = 0; i < 1; i++)
	{
		bool t = booksDB.begin_load_from_file_bin(files[i]);
		if(!t)
		{
			zdebug::log()->Log(zstr::fmt("%d/%d finished\n", (int)i, (int)files.size()));
			break;
		}
		//
		while(booksDB.to_next_book())
		{
			zBookInfo *book = booksDB.get_cur_book();
			if(dbProcs.is_bad_book(book->get_name()))	//плохая книга
				continue;

			const std::vector<int> &words = book->get_words();
			int cnt = (int)words.size();
			if(cnt < 3)
				continue;

			book_num++;

			bool cur_sent_bad = true;
			int cur_sent_ind = 0;
			//пробегаем все слова книги
			for(int ii = 0; ii < cnt; ii++)
			{
				int w = words[ii];
				if(w == BI_SENT_END)
				{
					if(!cur_sent_bad && cur_sent_ind+2 <= ii-1)
					{
						//хорошие предложения используем для статистики третьих слов
						//из трёх слов получается 2 пары, их обе мы проверяем на наличие в массиве пар
						//и если присутствуют - увеличиваем w3count_vec для первой пары

						int ind1 = _get_pair_ind( get_pair_from_words(words[cur_sent_ind], words[cur_sent_ind+1]) );
						
						for(int jj = cur_sent_ind+2; jj <= ii-1; jj++)
						{
							int ind2 = _get_pair_ind( get_pair_from_words(words[jj-1], words[jj]) );
							if(ind1 != INT_MAX && ind2 != INT_MAX)
							{
								w3count_vec[ind1]++;
								if(w3count_vec[ind1] == UINT_MAX)
									(rel_assert(0));
								dbg_added++;
							}
							else
								dbg_not_added++;

							ind1 = ind2;
						}
					}

					cur_sent_bad = true;
					cur_sent_ind = ii+1;
				}
				else
				{
					if(cur_sent_bad && !m_is_one_letter[w])
						cur_sent_bad = false;
				}
			}

			if(book_num % 500 == 0)
			{
				zdebug::log()->LogImp(zstr::fmt("%d book, dbg_added: %I64d, dbg_not_added: %I64d", book_num, dbg_added, dbg_not_added));
				//break;
			}
		}
	}

	zRWArray::save_vec_to_file(w3count_vec, L"PAIRS9/words3/words3.cnt");
	zdebug::log()->LogImp(zstr::fmt("calc_pairs_word3 end. dbg_added=%I64d. dbg_not_added=%I64d", dbg_added, dbg_not_added));

	return true;
}


//------------------------------------------------------------------------------------------
//та же функция, но записывает их в массив
bool zStatProcs::calc_pairs_word3_fill(int ind_first, int ind_last, unsigned int arr_size)
{
	zRWArray::read_array(pairs_vec, L"PAIRS9/best50kk.pairs", 0, INT_MAX);
	zRWArray::read_array(w3count_vec, L"PAIRS9/words3/words3.cnt", ind_first, ind_last);

	if(ind_last > (int)pairs_vec.size())
		return (rel_assert(0), false);

	zdebug::log()->LogImp(zstr::fmt("size is %d", (int)w3count_vec.size()));

	std::wstring new_fname = zstr::wfmt(L"PAIRS9/words3_res/%09d-%09d.w3db", ind_first, ind_last);
	zdebug::log()->LogImp("file name will be " + zstr::w_to_s(new_fname));

	//выделяем массив под w3-слова
	w3result_vec.clear();
	w3result_vec.resize(arr_size, -1);

	//проставляем индексы для записи w3count_vec
	for(int i = 1; i < (int)w3count_vec.size(); i++)
		w3count_vec[i] += w3count_vec[i-1];

	rel_assert(w3count_vec[w3count_vec.size()-1] == arr_size);

	//
	zBooksDB booksDB;
	zBooksDBProcs dbProcs;
	dbProcs.load_books_list(L"__books_list");

	_load_all_needed();
	_fill_is_one_letter();

	//
	int book_num = 0;
	__int64 dbg_added = 0, dbg_not_added = 0;

	__int64 val_min = pairs_vec[ind_first];
	__int64 val_max = pairs_vec[ind_last-1];

	//проверим на всякий случай отсортированность pairs_vec
	for(int ii = 1; ii < (int)pairs_vec.size(); ii++)
		rel_assert(pairs_vec[ii] > pairs_vec[ii-1]);

	const std::vector<std::wstring> &files = dbProcs.get_DB_files_list();
	for(size_t i = 0; i < files.size(); i++)
	//for(size_t i = 0; i < 1; i++)
	{
		bool t = booksDB.begin_load_from_file_bin(files[i]);
		if(!t)
		{
			zdebug::log()->Log(zstr::fmt("%d/%d finished\n", (int)i, (int)files.size()));
			break;
		}
		//
		while(booksDB.to_next_book())
		{
			zBookInfo *book = booksDB.get_cur_book();
			if(dbProcs.is_bad_book(book->get_name()))	//плохая книга
				continue;

			const std::vector<int> &words = book->get_words();
			int cnt = (int)words.size();
			if(cnt < 3)
				continue;

			book_num++;

			bool cur_sent_bad = true;
			int cur_sent_ind = 0;
			//пробегаем все слова книги
			for(int ii = 0; ii < cnt; ii++)
			{
				int w = words[ii];
				if(w == BI_SENT_END)
				{
					if(!cur_sent_bad && cur_sent_ind+2 <= ii-1)
					{
						//хорошие предложения используем для статистики третьих слов
						//из трёх слов получается 2 пары, их обе мы проверяем на наличие в массиве пар
						//и если присутствуют - увеличиваем w3count_vec для первой пары

						__int64 pair1 = get_pair_from_words(words[cur_sent_ind], words[cur_sent_ind+1]);
						int indPrev = INT_MAX;
						//if()
						{
							for(int jj = cur_sent_ind+2; jj <= ii-1; jj++)
							{
								int wjj = words[jj];
								__int64 pair2 = get_pair_from_words(words[jj-1], wjj);

								//если пара попадает в промежуток, значит это возможно наш клиент
								if(pair1 >= val_min && pair1 <= val_max)
								{
									int ind_1 = (indPrev == INT_MAX) ? _get_pair_ind(pair1) : indPrev;
									if(ind_1 < ind_first)
										return (rel_assert(0), false);

									if(ind_1 != INT_MAX)	//точно наш клиент
									{
										int ind_2 = _get_pair_ind(pair2);
										if(ind_2 != INT_MAX)
										{
											indPrev = ind_2;

											unsigned int &index = w3count_vec[ind_1-ind_first];
											rel_assert(index > 0);
											--index;
											w3result_vec[index] = wjj;
											dbg_added++;
										}
										else
										{
											dbg_not_added++;
											indPrev = INT_MAX;
										}
									}
									else
									{
										dbg_not_added++;
										indPrev = INT_MAX;
									}
								}
								else
									indPrev = INT_MAX;

								//ind1 = ind2;
								pair1 = pair2;
							}
						}
					}

					cur_sent_bad = true;
					cur_sent_ind = ii+1;
				}
				else
				{
					if(cur_sent_bad && !m_is_one_letter[w])
						cur_sent_bad = false;
				}
			}

			if(book_num % 500 == 0)
			{
				zdebug::log()->LogImp(zstr::fmt("%d book, dbg_added: %I64d, dbg_not_added: %I64d", book_num, dbg_added, dbg_not_added));
				_check_if_paused();

				//break;
			}
		}
	}

	rel_assert(dbg_added == arr_size);

	//тестируем, что в массиве нет пустых значений
	for(int i = 0; i < (int)arr_size; i++)
	{
		if(w3result_vec[i] == -1)
		{
			rel_assert(0);
			break;
		}
	}

	zRWArray::save_vec_to_file(w3result_vec, new_fname);
	zdebug::log()->LogImp(zstr::fmt("calc_pairs_word3_fill end. dbg_added=%I64d. dbg_not_added=%I64d", dbg_added, dbg_not_added));

	return true;
}


//------------------------------------------------------------------------------------------
//подсчитать длины подмассивов, на которые разобъем итоговый массив
void zStatProcs::test_best50kk_subarr_len()
{
	zRWArray::read_array(pairs_vec, L"PAIRS9/best50kk.pairs", 0, 999999999);
	zRWArray::read_array(w3count_vec, L"PAIRS9/words3/words3.cnt", 0, 999999999);

	if(pairs_vec.size() != w3count_vec.size())
		return (rel_assert(0));

	__int64 split_size = 100 * (1 << 20);

	int num = 0;
	__int64 cur_size = INT_MAX;
	for(int i = 0; i < (int)w3count_vec.size(); i++)
	{
		if(cur_size == INT_MAX)
		{
			zdebug::log()->Log(zstr::fmt("%02d. ind = %d", num++, i));
			cur_size = 0;
		}
		cur_size += w3count_vec[i];

		if(cur_size >= split_size)
		{
			zdebug::log()->Log(zstr::fmt("     to ind = %d. size = %I64d", i, cur_size));
			cur_size = INT_MAX;
		}
	}

	zdebug::log()->Log(zstr::fmt("     to ind = %d. size = %I64d", (int)w3count_vec.size(), cur_size));
}


//------------------------------------------------------------------------------------------
//ставит и снимает с паузы по Ctrl+Shift+RWin
void zStatProcs::_check_if_paused()
{
	bool bPaused = false;
	if(GetKeyState(VK_SHIFT) < 0 && GetKeyState(VK_CONTROL) < 0  && GetKeyState(VK_RWIN) < 0)
	{
		bPaused = true;
		zdebug::log()->LogImp("paused");
	}

	while(bPaused)
	{
		Sleep(1000);
		if(GetKeyState(VK_SHIFT) < 0 && GetKeyState(VK_CONTROL) < 0  && GetKeyState(VK_RWIN) < 0)
		{
			bPaused = false;
			zdebug::log()->LogImp("paused off");
		}
	}
}


//------------------------------------------------------------------------------------------
//составляет список пар, кот. являются первыми в предложениях
bool zStatProcs::find_first_pairs()
{
	zRWArray::read_array(pairs_vec, L"PAIRS9/best50kk.pairs", 0, INT_MAX);

	zdebug::log()->LogImp(zstr::fmt("size is %d", (int)pairs_vec.size()));

	//выделяем массив
	m_is_first_pair_.clear();
	m_is_first_pair_.resize(pairs_vec.size());

	//грузим zWordsDBFast
	zWordsDBFast words_db_fast;
	words_db_fast.load_from_file_bin(L"_DB_words.wdb");

	//
	zBooksDB booksDB;
	zBooksDBProcs dbProcs;
	dbProcs.load_books_list(L"__books_list");

	_load_all_needed();
	_fill_is_one_letter();

	//
	int book_num = 0;
	__int64 dbg_total_sent = 0, dbg_short_sent = 0, dbg_bad_pair_sent = 0, dbg_added_sent = 0;

	//
	const std::vector<std::wstring> &files = dbProcs.get_DB_files_list();
	for(size_t i = 0; i < files.size(); i++)
	//for(size_t i = 0; i < 1; i++)
	{
		bool t = booksDB.begin_load_from_file_bin(files[i]);
		if(!t)
		{
			zdebug::log()->Log(zstr::fmt("%d/%d finished\n", (int)i, (int)files.size()));
			break;
		}
		//
		while(booksDB.to_next_book())
		{
			zBookInfo *book = booksDB.get_cur_book();
			if(dbProcs.is_bad_book(book->get_name()))	//плохая книга
				continue;

			const std::vector<int> &words = book->get_words();
			int cnt = (int)words.size();
			if(cnt < 3)
				continue;

			book_num++;

			bool cur_sent_bad = true;
			int cur_sent_ind = 0;
			//пробегаем все слова книги
			for(int ii = 0; ii < cnt; ii++)
			{
				int w = words[ii];
				if(w == BI_SENT_END)
				{
					if(!cur_sent_bad)
					{
						dbg_total_sent++;
						if(cur_sent_ind+2 <= ii-1)
						{
							//хорошие предложения используем для статистики первых пар
							__int64 pair1 = get_pair_from_words(words[cur_sent_ind], words[cur_sent_ind+1]);
							int ind = _get_pair_ind(pair1);
							if(ind == INT_MAX)
								dbg_bad_pair_sent++;
							else
							{
								dbg_added_sent++;
								rel_assert(ind >= 0 && ind < (int)m_is_first_pair_.size());

								unsigned int &cur_val = m_is_first_pair_[ind];
								if(cur_val < UINT_MAX)
								{
									cur_val++;

									if(cur_val == UINT_MAX)
									{
										//rel_assert(0);
										int w1 = words[cur_sent_ind], w2 = words[cur_sent_ind+1];
										rel_assert(words_db_fast.is_valid_id(w1) && words_db_fast.is_valid_id(w2));

										const char *s1 = words_db_fast.get_str_by_id(w1);
										const char *s2 = words_db_fast.get_str_by_id(w2);
										zdebug::log()->Log(zstr::fmt("!!!popular first pair: %s %s", s1, s2));
									}
								}
							}
						}
						else
							dbg_short_sent++;
					}

					cur_sent_bad = true;
					cur_sent_ind = ii+1;
				}
				else
				{
					if(cur_sent_bad && !m_is_one_letter[w])
						cur_sent_bad = false;
				}
			}

			if(book_num % 500 == 0)
			{
				zdebug::log()->LogImp(zstr::fmt("%d book", book_num));
				_check_if_paused();

				//break;
			}
		}
	}

	//
	std::wstring new_fname = L"PAIRS9/first_pairs.dat";
	zRWArray::save_vec_to_file(m_is_first_pair_, new_fname);

	zdebug::log()->Log("find_first_pairs end");
	zdebug::log()->LogImp(zstr::fmt("dbg_total_sent=%I64d. dbg_short_sent=%I64d. dbg_bad_pair_sent=%I64d. dbg_added_sent=%I64d.", dbg_total_sent, dbg_short_sent, dbg_bad_pair_sent, dbg_added_sent));

	return true;
}


//------------------------------------------------------------------------------------------
void zStatProcs::_get_sent(const std::vector<int> &words, int first_ind, int last_ind, std::string &ret_str)
{
	ret_str.clear();
	for(int i = first_ind; i <= last_ind; i++)
	{
		int word = words[i];
		if(m_words_db.is_valid_id(word))
		{
			if(!ret_str.empty())
				ret_str += ' ';
			ret_str += m_words_db.get_str_by_id(word);
		}
		else
			(rel_assert(0));
	}
}


//------------------------------------------------------------------------------------------
//подсчитывает, сколько каждое слово бывает последним в предложениях
bool zStatProcs::find_last_words()
{
	//грузим zWordsDBFast
	zWordsDBFast words_db_fast;
	words_db_fast.load_from_file_bin(L"_DB_words.wdb");

	m_last_word.resize(words_db_fast.get_size());

	//
	zBooksDB booksDB;
	zBooksDBProcs dbProcs;
	dbProcs.load_books_list(L"__books_list");

	_load_all_needed();
	_fill_is_one_letter();

	//
	int book_num = 0;
	__int64 dbg_total_sent = 0, dbg_short_sent = 0, dbg_bad_pair_sent = 0, dbg_added_sent = 0;

	std::string temp_str;
	//
	const std::vector<std::wstring> &files = dbProcs.get_DB_files_list();
	//for(size_t i = 0; i < files.size(); i++)
	for(size_t i = 0; i < 1; i++)
	{
		bool t = booksDB.begin_load_from_file_bin(files[i]);
		if(!t)
		{
			zdebug::log()->Log(zstr::fmt("%d/%d finished\n", (int)i, (int)files.size()));
			break;
		}
		//
		while(booksDB.to_next_book())
		{
			zBookInfo *book = booksDB.get_cur_book();
			if(dbProcs.is_bad_book(book->get_name()))	//плохая книга
				continue;

			const std::vector<int> &words = book->get_words();
			int cnt = (int)words.size();
			if(cnt < 3)
				continue;

			book_num++;

			bool cur_sent_bad = true;
			int cur_sent_ind = 0;
			//пробегаем все слова книги
			for(int ii = 0; ii < cnt; ii++)
			{
				int w = words[ii];
				if(w == BI_SENT_END)
				{
					if(!cur_sent_bad)
					{
						dbg_total_sent++;
						if(cur_sent_ind+2 <= ii-1)
						{
							int last_word = words[ii-1];
							if(last_word != BI_SENT_END)
							{
								if(last_word == 0)	//откуда ж так много 'и' в конце?
								{
									_get_sent(words, cur_sent_ind, ii-1, temp_str);
									zdebug::log()->LogImp(temp_str);
								}

								rel_assert(words_db_fast.is_valid_id(last_word));

								unsigned short &val = m_last_word[last_word];
								if(val < USHRT_MAX)
								{
									val++;
									if(val == USHRT_MAX)
									{
										zdebug::log()->LogImp(zstr::fmt("m_last_word[i] == USHRT_MAX. <%s, %d>",
											m_words_db.get_str_by_id(last_word), last_word));
									}
								}
							}
							dbg_added_sent++;
						}
						else
							dbg_short_sent++;
					}

					cur_sent_bad = true;
					cur_sent_ind = ii+1;
				}
				else
				{
					if(cur_sent_bad && !m_is_one_letter[w])
						cur_sent_bad = false;
				}
			}

			if(book_num % 500 == 0)
			{
				zdebug::log()->LogImp(zstr::fmt("%d book", book_num));
				_check_if_paused();

				break;
			}
		}
	}

	//
	std::wstring new_fname = L"_DB_last_words.dat";
	zRWArray::save_vec_to_file(m_last_word, new_fname);

	zdebug::log()->Log("find_last_words end");
	zdebug::log()->LogImp(zstr::fmt("dbg_total_sent=%I64d. dbg_short_sent=%I64d. dbg_bad_pair_sent=%I64d. dbg_added_sent=%I64d.", dbg_total_sent, dbg_short_sent, dbg_bad_pair_sent, dbg_added_sent));

	return true;
}

