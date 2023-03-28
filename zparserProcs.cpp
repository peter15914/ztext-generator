#include "stdafx.h"

#include "zparser.h"

#include <zstlib/zstr.h>
#include <zstlib/utl/zLog.h>
#include <zstlib/zlua.h>

#include "zTextParser.h"
#include "zWordsDBFill.h"
#include "zWordsDB.h"
#include "zWordsDBFast.h"
#include "zBooksDB.h"
#include "zPairsDBFill.h"
#include "zBooksDBProcs.h"
#include "zStatProcs.h"
#include "zGenProcs.h"



//------------------------------------------------------------------------------------------
using namespace std;



//------------------------------------------------------------------------------------------
void _test_utf8()
{
	ifstream file1("fb2/104217.fb2");
	ofstream file2("fb2/104217.fb2.res");

	string s;
	while(getline(file1, s))
	{
		s = zstr::utf8_to_ansi(s);
		file2 << s << endl;
	}

	file1.close();
	file2.close();
}


//------------------------------------------------------------------------------------------
void _load_files_list(std::vector<std::wstring> &files)
{
	files.clear();

	std::ifstream files_list(L"fb2/_list");
	std::string s;
	while(files_list)
	{
		std::getline(files_list, s);
		if(!files_list)
			break;

		if(!s.empty())
			files.push_back(L"fb2/" + zstr::s_to_w(s));
	}
}


//------------------------------------------------------------------------------------------
void _fill_DB()
{
	std::wstring sDBFileName = L"_DB.txt";
	//std::wstring sDBFileName = L"_DB_test.txt";

	zWordsDB db;
	zdebug::log()->Log("begin load_from_file");
	if(zutl::FileExists(sDBFileName))
		db.load_from_file(sDBFileName);
	zdebug::log()->Log("end load_from_file, " + zdebug::log()->GetLogTimeSpend() + "\n");

	zWordsDBFill dbFill;
	dbFill.set_MinSentLen(4);

	//
	std::vector<std::wstring> files;
	_load_files_list(files);

	//
	size_t first = 0;
	size_t last =  99999;
	if(last >= files.size())
		last = files.empty() ? 0 : files.size()-1;

	//
	DWORD dParseBegin = timeGetTime();
	//
	size_t initSize = db.get_size();
	size_t prevSize = initSize;
	size_t maxSize = 0;
	//if(0)
	for(size_t i = first; i <= last; i++)
	{
		//!!опасная опция
		//dbFill.set_dec_mode(true);

		dbFill.add_words_from_file(&db, files[i], true);
		//cout << i << " / " << last << ". size = " << db.get_size() << endl;
		//cout << i << " / " << last << ". " << dbFill.get_stat_str();

		string file_name = zstr::w_to_s(files[i]);
		string s = zstr::fmt("%d. %s. ", i, file_name.c_str());
		zdebug::log()->Log(s + dbFill.get_stat_str() + ", " + zdebug::log()->GetLogTimeSpend());

		if(db.get_size() - prevSize > maxSize)
			maxSize = db.get_size() - prevSize;
		prevSize = db.get_size();
	}

	DWORD dd = timeGetTime() - dParseBegin;

	size_t fcnt = (last >= first) ? last-first+1 : 0;
	zdebug::log()->Log("\n" + zstr::fmt("total generation time (%u files): %lu.%03lu", fcnt, dd/1000, dd%1000));

	//
	zdebug::log()->Log("\nbegin save_to_file");
	db.save_to_file(sDBFileName);
	zdebug::log()->Log("end save_to_file, " + zdebug::log()->GetLogTimeSpend());
	//log_time_spend();

	//
	zdebug::log()->Log("total new words:" + zstr::fmt("%u", db.get_size() - initSize));
	zdebug::log()->Log("max new words from one file:" + zstr::fmt("%u", maxSize));
}


//------------------------------------------------------------------------------------------
void _clean_DB()
{
	std::wstring sDBFileName = L"_DB.txt";
	std::wstring sDBBooksCntFileName = L"_DB_books_cnt.txt";

	zWordsDB db;
	zdebug::log()->Log("begin load_from_file");
	db.load_from_file(sDBFileName);
	db.load_from_file_books_cnt(sDBBooksCntFileName);
	zdebug::log()->Log("end load_from_file, " + zdebug::log()->GetLogTimeSpend() + "\n");
	//

	ofstream out_file(L"_aaatemp.txt");
	//ofstream out_file2(L"_aaatemp2.txt");
	//разовая чистка
	std::vector<zWord> &vec = db.get_Data();
	for(size_t i = 0; i < vec.size(); i++)
	{
		string s = vec[i].m_str;
		int len = (int)s.length();
		int freq = vec[i].m_Frequency;

		//if(freq > 0 && len > 0 && (s[0] == 'ь' || s[0] == 'ы' || s[0] == 'ъ'))
		//if(freq > 0 && len > 0 && (s[len-1] == 'ъ'))
		if(freq > 0 && s.find('ъ') != string.npos)
		{
			//vec[i].m_Frequency = -999;
			out_file << s << " " << freq << "\n";
		}

		if(0)//freq > 0 && len > 1)
		{
			char c1 = s[0];
			char c2 = s[1];
			if(c1 == 'я' && (c2 == 'я' || c2 == 'ю' || c2 == 'е'))
			{
				vec[i].m_Frequency = -999;
				//out_file << s << " " << freq << "\n";
			}
		}

		if(0)//!db.is_rare_or_bad_word(i))
		{
			out_file << s << "\n";
		}

		//for(int j = 0; j < len-3; j++)
		//{
		//	if(s[j] == s[j+1] && s[j+1] == s[j+2])
		//	{
		//		//out_file << s << "\n";
		//		if(s[j] != 'е' && (s[j] != 'с' || j+3 < len && s[j+3] == 'р'))	//'еее' или 'ссср'
		//			vec[i].m_Frequency = 0;
		//		break;
		//	}
		//}

		//if(len >= 25 && s.find('-') == string.npos)
		//{
		//	if(freq <= 10)
		//		vec[i].m_Frequency = 0;
		//	//	out_file << s << " " << freq << "\n";
		//	//else
		//	//	out_file2 << s << " " << freq << "\n";
		//}
	}

	out_file.close();
	//out_file2.close();

	int buf = db.get_bad_or_rare_words_count();
	zdebug::log()->Log(zstr::fmt("bad or rare words count: %d", buf));
	zdebug::log()->Log(zstr::fmt("normal words count: %d", db.get_size() - buf));

	//
	if(0)
	{
		zdebug::log()->Log("\nbegin save_to_file");
		db.save_to_file(sDBFileName);
		zdebug::log()->Log("end save_to_file, " + zdebug::log()->GetLogTimeSpend());
	}
}


//------------------------------------------------------------------------------------------
void _test()
{
	zTextParser parser;
	parser.open(L"fb2/10001.fb2", true);
	parser.test_write_to_file(L"__1111.txt");
}


//------------------------------------------------------------------------------------------
void _read_books(size_t first, size_t last, int var)
{
	std::wstring sDBFileName = L"_DB.txt";
	std::wstring sBooksDBFileName = L"_test_books.txt" + zstr::wfmt(L"%d", var);
	std::wstring sBooksDBFileNameBin = L"_test_books.db" + zstr::wfmt(L"%d", var);

	zWordsDB word_db;
	zdebug::log()->Log("begin load_from_file");
	if(zutl::FileExists(sDBFileName))
		word_db.load_from_file(sDBFileName);
	zdebug::log()->Log("end load_from_file, " + zdebug::log()->GetLogTimeSpend() + "\n");

	zWordsDBFill dbFill;
	dbFill.set_MinSentLen(4);

	zBooksDB booksDB;
	booksDB.set_wordsDB(&word_db);

	//
	std::vector<std::wstring> files;
	_load_files_list(files);

	//
	if(last >= files.size())
		last = files.empty() ? 0 : files.size()-1;


	for(size_t i = first; i <= last; i++)
	{
		dbFill.add_book_from_file(&booksDB, &word_db, files[i], true);

		string file_name = zstr::w_to_s(files[i]);
		string s = zstr::fmt("%d. %s. ", i, file_name.c_str());
		zdebug::log()->Log(s + dbFill.get_stat_str() + ", " + zdebug::log()->GetLogTimeSpend());

		const std::string &bad_str = dbFill.get_bad_str();
		if(!bad_str.empty())
		{
			ofstream out_file((L"_bad_temp/"+files[i]).c_str());
			out_file << bad_str;
			out_file.close();
		}
	}

	zdebug::log()->Log("begin booksDB.save_to_file");
	booksDB.save_to_file(sBooksDBFileName);
	booksDB.save_to_file_bin(sBooksDBFileNameBin);
	zdebug::log()->Log("end booksDB.save_to_file, " + zdebug::log()->GetLogTimeSpend() + "\n");
}


//------------------------------------------------------------------------------------------
void _test_load_from_file_bin(int var)
{
	std::wstring sDBFileName = L"_DB.txt";
	std::wstring sBooksDBFileName = L"___temp__test_books.txt" + zstr::wfmt(L"%d", var);
	std::wstring sBooksDBFileNameBin = L"_test_books.db" + zstr::wfmt(L"%d", var);

	zWordsDB word_db;
	//zdebug::log()->Log("begin load_from_file");
	//if(zutl::FileExists(sDBFileName))
	//	word_db.load_from_file(sDBFileName);
	//zdebug::log()->Log("end load_from_file, " + zdebug::log()->GetLogTimeSpend() + "\n");

	//zWordsDBFill dbFill;
	//dbFill.set_MinSentLen(4);

	zBooksDB booksDB;
	booksDB.set_wordsDB(&word_db);

	zdebug::log()->Log("begin booksDB.load_from_file_bin");
	booksDB.load_from_file_bin(sBooksDBFileNameBin);
	zdebug::log()->Log("end booksDB.load_from_file_bin, " + zdebug::log()->GetLogTimeSpend() + "\n");

	zdebug::log()->Log("begin booksDB.save_to_file");
	booksDB.save_to_file(sBooksDBFileName);
	zdebug::log()->Log("end booksDB.save_to_file, " + zdebug::log()->GetLogTimeSpend() + "\n");
}


//------------------------------------------------------------------------------------------
void _test_02()
{
	std::wstring sBooksDBFileNameBin = L"_books_300000-308999.db";

	zBooksDB booksDB;

	zdebug::log()->ResetTime();
	zdebug::log()->Log("begin booksDB.load_from_file_bin");
	booksDB.load_from_file_bin(sBooksDBFileNameBin);
	zdebug::log()->Log("end booksDB.load_from_file_bin, " + zdebug::log()->GetLogTimeSpend() + "\n");
}


//------------------------------------------------------------------------------------------
void _test_03()
{
	//zPairsDB pairsDB;
	//for(int i = 0; i < 100000; i++)
	//{
	//	int dd = 1+i*5;
	//	pairsDB.add_tripple(2, 5, rand()%dd, false);
	//	pairsDB.add_tripple(1, 2, rand()%dd, false);
	//	pairsDB.add_tripple(1, 3, rand()%dd, false);
	//	pairsDB.add_tripple(1, 2, rand()%dd, false);
	//	pairsDB.add_tripple(1, 3, rand()%dd, false);
	//	pairsDB.add_tripple(1, 4, rand()%dd, false);
	//	pairsDB.add_tripple(1, 4, rand()%dd, false);
	//	pairsDB.add_tripple(2, 6, rand()%dd, false);
	//	pairsDB.add_tripple(2, 5, rand()%dd, false);
	//}

	//MessageBox(0, "000", "000", MB_OK);
	//zdebug::log()->Log("pairsDB stats: " + pairsDB.get_stat_str());
}


//------------------------------------------------------------------------------------------
void _test_04()
{
	std::wstring sBooksDBFileNameBin = L"BOOKS_DB/_books_300000-308999.db";

	zBooksDB booksDB;

	zdebug::log()->ResetTime();
	zdebug::log()->Log("begin booksDB.load_from_file_bin");
	booksDB.load_from_file_bin(sBooksDBFileNameBin);
	zdebug::log()->Log("end booksDB.load_from_file_bin, " + zdebug::log()->GetLogTimeSpend() + "\n");

	//booksDB.save_to_file(L"_____test01.txt");
	//booksDB.clear();

	//
	zBooksDB booksDB2;

	zdebug::log()->ResetTime();
	zdebug::log()->Log("begin booksDB.begin_load_from_file_bin");

	if(booksDB2.begin_load_from_file_bin(sBooksDBFileNameBin))
	{
		int ii = 0;
		while(booksDB2.to_next_book())
		{
			zBookInfo *book = booksDB2.get_cur_book();
			zBookInfo *book2 = booksDB.get_book_by_id(ii++);

			if(book->get_words() != book2->get_words())
				zdebug::log()->Log("!!!!!!!!!!!!!!!!!!!!!!!!!!!BAD");
		}
	}

	zdebug::log()->Log("end , " + zdebug::log()->GetLogTimeSpend() + "\n");
}


//------------------------------------------------------------------------------------------
void _test_fill_pairs_db()
{
	//MessageBox(0, "000", "000", MB_OK);
	//std::wstring sBooksDBFileNameBin = L"BOOKS_DB/_books_300000-308999.db";

	//zBooksDB booksDB;

	//zdebug::log()->ResetTime();
	//zdebug::log()->Log("begin booksDB.load_from_file_bin");
	//booksDB.load_from_file_bin(sBooksDBFileNameBin);
	//zdebug::log()->Log("end booksDB.load_from_file_bin, " + get_log_time_spend() + "\n");

	//zPairsDB pairsDB;
	//zPairsDBFill pairsDBFill;

	//MessageBox(0, "111", "111", MB_OK);
	//zdebug::log()->Log("begin add_pairs_from_books_db, " + get_log_time_spend() + "\n");
	//pairsDBFill.add_pairs_from_books_db(&pairsDB, &booksDB);
	//zdebug::log()->Log("end add_pairs_from_books_db, " + get_log_time_spend() + "\n");

	//zdebug::log()->Log("pairsDB stats: " + pairsDB.get_stat_str());

	//MessageBox(0, "aaa", "bbb", MB_OK);

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

	std::set<__int64> pairs;
	int book_num = 0;
	int added_total = 0;
	//
	zBooksDB booksDB;
	const std::vector<std::wstring> &files = dbProcs.get_DB_files_list();
	for(size_t i = 0; i < files.size(); i++)
	//for(size_t i = 0; i < 1; i++)
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
								pairs.insert(buf);
								added_total++;
							}
						}
					}

					if(book_num % 100 == 0)
					{
						zdebug::log()->LogImp(zstr::fmt("%d book, size: %d, added_total: %d", book_num, (int)pairs.size(), added_total));
					}
				}

			}
		}
		zdebug::log()->Log(zstr::fmt("%d/%d finished\n", (int)i, (int)files.size()));
	}

	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");

}


//------------------------------------------------------------------------------------------
void output_book(std::string sBook2Read, std::wstring sResFile)
{
	//std::wstring sDBFileName = L"_DB.txt";
	std::wstring sDBFileNameBin = L"_DB_words.wdb";

	zWordsDBFast db;
	zdebug::log()->Log("begin load_from_file");
	db.load_from_file_bin(sDBFileNameBin);
	zdebug::log()->Log("end load_from_file, " + zdebug::log()->GetLogTimeSpend() + "\n");

	zBooksDBProcs dbProcs;
	dbProcs.load_books_list(L"__books_list");

	//---
	zBookPlace zz = dbProcs.get_book_place(sBook2Read);
	if(zz.m_db_num == INT_MAX || zz.m_offset == INT_MAX)
		return (rel_assert(0));

	//
	zBooksDB booksDB;
	booksDB.set_wordsDB(&db);

	zdebug::log()->ResetTime();
	zdebug::log()->Log("begin output_book");

	if(booksDB.load_book_by_offset(dbProcs.get_dbfile_name(zz.m_db_num), zz.m_offset))
	{
		zBookInfo *book = booksDB.get_cur_book();
		if(book && book->get_name() == sBook2Read)
		{
			FILE *pFile = _wfopen(sResFile.c_str(), L"w");
			booksDB.save_to_file(pFile, book);
			fclose(pFile);
		}
		else
			rel_assert(0);
	}

	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");
}


//------------------------------------------------------------------------------------------
//проверка скорости чтения всей базы книг
void test_bases_read_speed()
{
	zBooksDBProcs dbProcs;

	zdebug::log()->ResetTime();
	zdebug::log()->Log("begin test_bases_read_speed()");

	const std::vector<std::wstring> &files = dbProcs.get_DB_files_list();

	zBooksDB booksDB2;
	booksDB2.set_only_headers(true);

	for(size_t i = 0; i < files.size(); i++)
	{
		if(booksDB2.begin_load_from_file_bin(files[i]))
		{
			while(booksDB2.to_next_book())
			{
				zBookInfo *book = booksDB2.get_cur_book();
			}
		}
		zdebug::log()->Log(zstr::fmt("%d/%d finished\n", (int)i, (int)files.size()));
	}

	zdebug::log()->Log("end , " + zdebug::log()->GetLogTimeSpend() + "\n");
}


//------------------------------------------------------------------------------------------
void test_wordsDB_read_speed()
{
	std::wstring sDBFileName = L"_DB.txt";
	std::wstring sDBFileNameBin = L"_DB_words.wdb";

	zWordsDB db;
	zWordsDBFast db_fast;

	zdebug::log()->ResetTime();
	zdebug::log()->Log("begin test_wordsDB_read_speed()");

	db.load_from_file(sDBFileName);
	//db_fast.load_from_file_bin(sDBFileNameBin);

	//rel_assert(db.get_size() == db_fast.get_size());
	//int dbg_cnt = 0;
	//for(int i = 0; i < db_fast.get_size(); i++)
	//{
	//	rel_assert(db.get_word_by_id(i).m_str == db_fast.get_word_by_id(i));
	//	dbg_cnt++;
	//}
	//zdebug::log()->Log(zstr::fmt("%d words checked", dbg_cnt));

	zdebug::log()->Log("end , " + zdebug::log()->GetLogTimeSpend() + "\n");

	//db.save_to_file_bin(sDBFileNameBin);
}


//------------------------------------------------------------------------------------------
void process_01()
{
	std::wstring sDBFileName = L"_DB.txt";
	std::wstring sDBBooksCntFileName = L"_DB_books_cnt.txt";

	zWordsDB words_db;

	zdebug::log()->ResetTime();
	zdebug::log()->Log("begin words_db load\n");
	words_db.load_from_file(sDBFileName);
	words_db.load_from_file_books_cnt(sDBBooksCntFileName);
	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");

	int buf = words_db.get_bad_or_rare_words_count();
	zdebug::log()->Log(zstr::fmt("bad or rare words count: %d", buf));
	zdebug::log()->Log(zstr::fmt("normal words count: %d", words_db.get_size() - buf));

	//---------пересохранить базу
	//zBooksDBProcs dbProcs;

	//zdebug::log()->ResetTime();
	//zdebug::log()->Log("begin");

	////dbProcs.create_books_list();
	//dbProcs.load_books_list(L"/__books_list");
	//dbProcs.save_books_list(L"/__books_list2");

	//zdebug::log()->Log("end , " + zdebug::log()->GetLogTimeSpend() + "\n");

	//---------вывести все плохие книги со статистикой
	//zBooksDBProcs dbProcs;
	//dbProcs.load_books_list(L"__books_list");

	//zBooksDB booksDB;
	//booksDB.set_only_headers(true);

	//const zBookPlaceMap &data = dbProcs.get_all_books();
	//int ii = 0;
	//for(zBookPlaceMap::const_iterator it = data.begin(); it != data.end(); it++)
	//{
	//	std::string book_name = it->first;
	//	if(dbProcs.is_bad_book(book_name))
	//	{
	//		ii++;

	//		zdebug::log()->Log(zstr::fmt("%d. %s is bad", ii, book_name.c_str()));

	//		zBookPlace zz = dbProcs.get_book_place(book_name);
	//		if(zz.m_db_num == INT_MAX || zz.m_offset == INT_MAX)
	//			return (rel_assert(0));

	//		zBookInfo temp_book;
	//		temp_book.set_snt_stat(zz.m_snt_added, zz.m_snt_wrong_words, zz.m_snt_wrong_len);
	//		temp_book.set_name(book_name);

	//		zdebug::log()->Log(zstr::fmt("%s", temp_book.get_stat_str().c_str()));
	//		double dd = temp_book.get_good_percentage();
	//		zdebug::log()->Log(zstr::fmt("%.3f\n", dd));

	//		//if(booksDB.load_book_by_offset(dbProcs.get_dbfile_name(zz.m_db_num), zz.m_offset))
	//		//{
	//		//	zBookInfo *book = booksDB.get_cur_book();
	//		//	zdebug::log()->Log(zstr::fmt("%s", book->get_stat_str().c_str()));

	//		//	double dd = book->get_good_percentage();
	//		//	zdebug::log()->Log(zstr::fmt("%.3f\n", dd));
	//		//}
	//	}
	//}
}


//------------------------------------------------------------------------------------------
void output_bad_books()
{
	zBooksDBProcs dbProcs;
	dbProcs.load_books_list(L"__books_list");

	zdebug::log()->ResetTime();
	zdebug::log()->Log("begin output_bad_books()");

	//zdebug::log()->SetWriteToCout(false);

	const zBookPlaceMap &data = dbProcs.get_all_books();
	int ii = 0;
	for(zBookPlaceMap::const_iterator it = data.begin(); it != data.end(); it++)
	{
		std::string book_name = it->first;
		if(!dbProcs.is_bad_book(book_name))
		{
			zBookPlace zz = dbProcs.get_book_place(book_name);
			//rel_assert(zz.m_snt_added + zz.m_snt_wrong_words + zz.m_snt_wrong_len > 0);

			bool bad = false;
			double dd = 0.0;

			//if(zz.m_snt_added == 0)
			//	bad = true;
			if(zz.m_snt_added > 0)
			{
				zBookInfo temp_book;
				temp_book.set_snt_stat(zz.m_snt_added, zz.m_snt_wrong_words, zz.m_snt_wrong_len);

				dd = temp_book.get_good_percentage();

				if(dd <= 0.25)
					bad = true;
			}

			//выводим
			if(bad)
			{
				ii++;
				zdebug::log()->Log(book_name.c_str());
				//zdebug::log()->Log(zstr::fmt("%d. bad book: %.3f", ii, dd));

				std::string fileSrc = "F:/zm/zwriter_wrk/_bad_temp/" + book_name;
				std::string fileDest = "F:/zm/zwriter_wrk/_bad_temp/bad_0.25/" + book_name;
				//BOOL bufret = MoveFileA(fileSrc.c_str(), fileDest.c_str());
				//if(!bufret)
				//	zdebug::log()->Log("!!!error moving file " + book_name);
			}
		}
	}

	zdebug::log()->SetWriteToCout(true);
	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");
}


//------------------------------------------------------------------------------------------
void calc_words_count()
{
	std::wstring sDBFileName = L"_DB.txt";
	//std::wstring sDBFileNameBin = L"_DB_words.wdb";
	std::wstring sDBBadFileName = L"_DB_bad_words.txt";
	std::wstring sDBBooksCntFileName = L"_DB_books_cnt.txt";

	zBooksDBProcs dbProcs;
	dbProcs.load_books_list(L"__books_list");

	//zWordsDBFast words_db;
	zWordsDB words_db;
	zdebug::log()->Log("begin load_from_file_bin");
	//words_db.load_from_file_bin(sDBFileNameBin);
	words_db.load_from_file(sDBFileName);
	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");

	//zWordsDB bad_words;

	int words_cnt = words_db.get_size();
	//std::vector<int> inBooksCnt(words_cnt, 0);
	std::vector<int> &books_cnt_arr = words_db.get_books_cnt_arr();
	std::vector<bool> curBook(words_cnt, false);

	zdebug::log()->ResetTime();
	zdebug::log()->Log("begin calc_words_count()");

	zBooksDB booksDB;
	const std::vector<std::wstring> &files = dbProcs.get_DB_files_list();
	for(size_t i = 0; i < files.size(); i++)
	//for(size_t i = 0; i < 1; i++)
	{
		if(booksDB.begin_load_from_file_bin(files[i]))
		{
			while(booksDB.to_next_book())
			{
				zBookInfo *book = booksDB.get_cur_book();
				//if(dbProcs.is_bad_book(book->get_name()))	//плохая книга
				if(!dbProcs.is_bad_book(book->get_name()))	//хорошая книга
				{
					const std::vector<int> &words = book->get_words();
					for(size_t ii = 0; ii < words.size(); ii++)
					{
						int wrd_id = words[ii];
						if(wrd_id != BI_SENT_END)
						{
							//bad_words.add_word(words_db.get_str_by_id(wrd_id));

							curBook[wrd_id] = true;
						}
					}
				}

				for(int jj = 0; jj < words_cnt; jj++)
				{
					if(curBook[jj])
					{
						//inBooksCnt[jj]++;
						books_cnt_arr[jj]++;
						curBook[jj] = false;
					}
				}
			}
		}
		zdebug::log()->Log(zstr::fmt("%d/%d finished\n", (int)i, (int)files.size()));
	}

	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");

	words_db.save_to_file_books_cnt(sDBBooksCntFileName);

	//bad_words.save_to_file(sDBBadFileName.c_str());
}


//------------------------------------------------------------------------------------------
void output_words_low_books()
{
	std::wstring sDBFileName = L"_DB.txt";
	std::wstring sDBBooksCntFileName = L"_DB_books_cnt.txt";
	std::wstring sDBFileNameBin = L"_DB_words.wdb";

	zWordsDB words_db;
	zWordsDBFast words_db_fast;

	zdebug::log()->Log("begin load_from_file");
	words_db.load_from_file_books_cnt(sDBBooksCntFileName);
	words_db.load_from_file(sDBFileName);
	//words_db_fast.load_from_file_bin(sDBFileNameBin);
	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");

	//
	std::vector<int> &books_cnt_arr = words_db.get_books_cnt_arr();

	zdebug::log()->Log("words in less or equal than 1 books:\n");
	zdebug::log()->SetWriteToCout(false);

	for(int ii = 0; ii < (int)books_cnt_arr.size(); ii++)
	{
		if(books_cnt_arr[ii] <= 10)
		//if(books_cnt_arr[ii] > 9 && books_cnt_arr[ii] <= 10)
		{
			zWord &wrd = words_db.get_word_by_id(ii);
			//if(wrd.m_Frequency > 300)
			zdebug::log()->Log(zstr::fmt("%s(%d)", wrd.m_str.c_str(), wrd.m_Frequency));
		}
	}
	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");
}


//------------------------------------------------------------------------------------------
void output_bad_or_good_words()
{
	std::wstring sDBFileName = L"_DB.txt";
	std::wstring sDBBooksCntFileName = L"_DB_books_cnt.txt";

	zWordsDB words_db;

	zdebug::log()->ResetTime();
	zdebug::log()->Log("begin words_db load\n");
	words_db.load_from_file(sDBFileName);
	words_db.load_from_file_books_cnt(sDBBooksCntFileName);
	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");

	int buf = words_db.get_bad_or_rare_words_count();
	zdebug::log()->Log(zstr::fmt("bad or rare words count: %d", buf));
	zdebug::log()->Log(zstr::fmt("normal words count: %d", words_db.get_size() - buf));

	//выводим все плохие или хорошие слова
	zdebug::log()->SetWriteToCout(false);
	std::vector<zWord> &vec2 = words_db.get_Data();
	for(int i = 0; i < (int)vec2.size(); i++)
	{
		//if(vec2[i].m_Frequency <= 0)
		if(words_db.is_rare_or_bad_word(i))
			;
		else
			zdebug::log()->Log(vec2[i].m_str);
	}
	zdebug::log()->SetWriteToCout(true);
}


//------------------------------------------------------------------------------------------
void dec_bad_words()
{
	std::wstring sDBFileName = L"_DB.txt";
	std::wstring sDBBadFileName = L"_DB_bad_words.txt";

	zWordsDB words_db;
	zWordsDB bad_words;

	std::vector<zWord> &vec = bad_words.get_Data();
	for(int i = 0; i < (int)vec.size(); i++)
	{
		zWord &bad_wrd = vec[i];

		const char *s = bad_wrd.m_str.c_str();
		int bad_freq = bad_wrd.m_Frequency;

		int id = words_db.get_word_id(s);
		zWord &word = words_db.get_word_by_id(id);
		word.m_Frequency -= bad_freq;
	}

	//
	zdebug::log()->Log("begin words_db save\n");
	words_db.save_to_file(L"_DB_2.txt");
	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");
}


//------------------------------------------------------------------------------------------
void find_books_rare_words()
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

	int words_cnt = words_db.get_size();

	//
	zdebug::log()->ResetTime();
	zdebug::log()->Log("begin find_books_rare_words()");

	//
	zBooksDB booksDB;
	const std::vector<std::wstring> &files = dbProcs.get_DB_files_list();
	for(size_t i = 0; i < files.size(); i++)
	//for(size_t i = 0; i < 1; i++)
	{
		if(booksDB.begin_load_from_file_bin(files[i]))
		{
			while(booksDB.to_next_book())
			{
				zBookInfo *book = booksDB.get_cur_book();
				if(!dbProcs.is_bad_book(book->get_name()))	//хорошая книга
				{
					//int some_words = 0;
					//int total_words = 0;
					int cnt_999 = 0;

					//пробегаем все слова книги
					const std::vector<int> &words = book->get_words();
					for(size_t ii = 0; ii < words.size(); ii++)
					{
						int wrd_id = words[ii];
						if(wrd_id != BI_SENT_END)
						{
							if(words_db.get_word_by_id(wrd_id).m_Frequency == -999)
							{
								cnt_999++;
							}
							//total_words++;
							//if(words_db.is_rare_or_bad_word(wrd_id) || words_db.get_word_by_id(wrd_id).m_Frequency <= 30)
							//	some_words++;
						}
					}

					if(cnt_999 >= 100)
					{
						zdebug::log()->Log(zstr::fmt("%s (%d)", book->get_name().c_str(), cnt_999));
					}

					//устанавливаем
					//zBookPlace &book_place = dbProcs.get_book_place2(book->get_name());
					//book_place.m_total_words = total_words;
					//book_place.m_rare_or_bad_words = some_words;
				}

			}
		}
		zdebug::log()->Log(zstr::fmt("%d/%d finished\n", (int)i, (int)files.size()));
	}

	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");

	//сохраним
	//dbProcs.save_books_list(L"/__books_list2");
}


//------------------------------------------------------------------------------------------
void output_books_with_rare_words()
{
	zBooksDBProcs dbProcs;
	dbProcs.load_books_list(L"__books_list");

	//
	zdebug::log()->ResetTime();
	zdebug::log()->Log("begin");

	//
	const zBookPlaceMap &data = dbProcs.get_all_books();
	for(zBookPlaceMap::const_iterator it = data.begin(); it != data.end(); it++)
	{
		std::string book_name = it->first;
		if(!dbProcs.is_bad_book(book_name))
		{
			zBookPlace zz = dbProcs.get_book_place(book_name);
			if(!zz.m_total_words)
				rel_assert(0);
			else
			{
				double pp = (double)zz.m_rare_or_bad_words / (double)zz.m_total_words;
				//if(pp >= 0.1)
				if(pp >= 0.05 && pp <= 0.1)
				{
					std::string s = zstr::fmt("%s(%d, %d)", book_name.c_str(), zz.m_total_words, zz.m_rare_or_bad_words);
					zdebug::log()->Log(s);
				}
			}
		}
	}

	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");
}


//------------------------------------------------------------------------------------------
void recalc_words_freqs()
{
	zdebug::log()->Log("begin recalc_words_freqs()");
	zWordsDB words_db1;
	zWordsDB words_db2;
	words_db1.load_from_file(L"_DB.txt");
	words_db2.load_from_file(L"__temp_DB_newfreqs.txt");
	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");

	//
	zdebug::log()->SetWriteToCout(false);
	int words_cnt_ = words_db1.get_size();
	for(int i = 0; i < words_cnt_; i++)
	{
		zWord &word1 = words_db1.get_word_by_id(i);
		zWord &word2 = words_db2.get_word_by_id(i);

		rel_assert(word1.m_str == word2.m_str);

		//можем только понижать
		if(word1.m_Frequency > 10 && word1.m_Frequency > word2.m_Frequency)
		{
			zdebug::log()->Log(zstr::fmt("%s(%d,%d)", word1.m_str.c_str(), word1.m_Frequency, word2.m_Frequency));
			word1.m_Frequency = word2.m_Frequency;
		}
	}

	words_db1.save_to_file(L"__temp_DB_newfreqs2.txt");

return;
	std::wstring sDBFileName = L"_DB.txt";
	std::wstring sDBFileNameBin = L"_DB_words.wdb";
	std::wstring sDBBooksCntFileName = L"_DB_books_cnt.txt";

	zBooksDBProcs dbProcs;
	dbProcs.load_books_list(L"__books_list");

	zWordsDB words_db;
	zWordsDBFast words_db_fast;
	zdebug::log()->Log("begin load_from_file_bin");
	words_db.load_from_file_books_cnt(sDBBooksCntFileName);
	words_db.load_from_file(sDBFileName);
	words_db_fast.load_from_file_bin(sDBFileNameBin);
	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");

	int words_cnt = words_db.get_size();
	std::vector<int> new_freqs(words_cnt);

	zdebug::log()->ResetTime();
	zdebug::log()->Log("begin calc_words_count()");

	zBooksDB booksDB;
	const std::vector<std::wstring> &files = dbProcs.get_DB_files_list();
	for(size_t i = 0; i < files.size(); i++)
	//for(size_t i = 0; i < 1; i++)
	{
		if(booksDB.begin_load_from_file_bin(files[i]))
		{
			while(booksDB.to_next_book())
			{
				zBookInfo *book = booksDB.get_cur_book();
				if(!dbProcs.is_bad_book(book->get_name()))	//хорошая книга
				{
					const std::vector<int> &words = book->get_words();
					for(size_t ii = 0; ii < words.size(); ii++)
					{
						int wrd_id = words[ii];
						if(wrd_id != BI_SENT_END && wrd_id >= 0)
						{
							if(wrd_id < words_cnt)
								new_freqs[wrd_id]++;
							else
								(rel_assert(0));
						}
					}
				}
			}
		}
		zdebug::log()->Log(zstr::fmt("%d/%d finished\n", (int)i, (int)files.size()));
	}

	zdebug::log()->Log("end, " + zdebug::log()->GetLogTimeSpend() + "\n");

	zdebug::log()->SetWriteToCout(false);
	for(int i = 0; i < words_cnt; i++)
	{
		int &cur_freq = words_db.get_word_by_id(i).m_Frequency;

		if(cur_freq != -999 && cur_freq > 0)
		{
			int new_freq = new_freqs[i];
			if(cur_freq == new_freq || cur_freq <= 50 && new_freq <= 50)
				;
			else
			{
				//если отличаются больше чем на 20%
				int pp = abs((new_freq-cur_freq) * 100 / max(new_freq, cur_freq));
				if(pp >= 20)
					zdebug::log()->Log(zstr::fmt("%s(%d,%d)", words_db.get_word_by_id(i).m_str.c_str(), cur_freq, new_freq));
				cur_freq = new_freq;
			}
		}
	}
	zdebug::log()->SetWriteToCout(true);

	words_db.save_to_file(L"__temp_DB_newfreqs.txt");
}


//------------------------------------------------------------------------------------------
void merge_pairs()
{
	//zStatProcs::merge_pairs_files(L"PAIRS/021_019", L"PAIRS/021_019_temp", L"PAIRS2/___ttt");

	//zStatProcs::create_pairs_stat_file();
	//zStatProcs::merge_pairs_files(L"PAIRS/000_001", L"PAIRS/001_002", L"PAIRS2/02_000_001");
	//zStatProcs::merge_pairs_files(L"PAIRS/003_001", L"PAIRS/004_002", L"PAIRS2/02_003_004");
	//zStatProcs::merge_pairs_files(L"PAIRS/005_003", L"PAIRS/007_004", L"PAIRS2/02_005_007");
	//zStatProcs::merge_pairs_files(L"PAIRS/008_005", L"PAIRS/010_006", L"PAIRS2/02_008_010");
	//zStatProcs::merge_pairs_files(L"PAIRS/011_007", L"PAIRS/013_008", L"PAIRS2/02_011_013");
	//zStatProcs::merge_pairs_files(L"PAIRS/014_009", L"PAIRS/015_010", L"PAIRS2/02_014_015");
	//zStatProcs::merge_pairs_files(L"PAIRS/016_011", L"PAIRS/017_012", L"PAIRS2/02_016_017");
	//zStatProcs::merge_pairs_files(L"PAIRS/017_013", L"PAIRS/018_014", L"PAIRS2/02_017_018");
	//zStatProcs::merge_pairs_files(L"PAIRS/019_015", L"PAIRS/019_016", L"PAIRS2/02_019_019");
	//zStatProcs::merge_pairs_files(L"PAIRS/020_017", L"PAIRS/020_018", L"PAIRS2/02_020_020");

	//zStatProcs::merge_pairs_files(L"PAIRS/021_019", L"PAIRS/021_020", L"PAIRS2/02_021_021");
	//zStatProcs::merge_pairs_files(L"PAIRS/022_021", L"PAIRS/024_022", L"PAIRS2/02_022_024");
	//zStatProcs::merge_pairs_files(L"PAIRS/024_023", L"PAIRS/025_024", L"PAIRS2/02_024_025");
	//zStatProcs::merge_pairs_files(L"PAIRS/026_001", L"PAIRS/026_025", L"PAIRS2/02_026_026");

	//zStatProcs::merge_pairs_files(L"PAIRS/026_026", L"PAIRS/027_002", L"PAIRS2/02_026_027");
	//zStatProcs::merge_pairs_files(L"PAIRS/027_003", L"PAIRS/028_004", L"PAIRS2/02_027_028");
	//zStatProcs::merge_pairs_files(L"PAIRS/029_005", L"PAIRS/029_006", L"PAIRS2/02_029_029");
	//zStatProcs::merge_pairs_files(L"PAIRS/030_007", L"PAIRS/031_008", L"PAIRS2/02_030_031");

	//zStatProcs::merge_pairs_files(L"PAIRS/032_009", L"PAIRS/032_010", L"PAIRS2/02_032_032");
	//zStatProcs::merge_pairs_files(L"PAIRS/033_011", L"PAIRS/034_012", L"PAIRS2/02_033_034");
	//zStatProcs::merge_pairs_files(L"PAIRS/035_013", L"PAIRS/036_014", L"PAIRS2/02_035_036");
	//zStatProcs::merge_pairs_files(L"PAIRS/036_015", L"PAIRS/037_016", L"PAIRS2/02_036_037");

	//zStatProcs::merge_pairs_files(L"PAIRS/038_017", L"PAIRS/038_018", L"PAIRS2/02_038_038");
	//zStatProcs::merge_pairs_files(L"PAIRS/039_019", L"PAIRS/040_020", L"PAIRS2/02_039_040");
	//zStatProcs::merge_pairs_files(L"PAIRS/041_021", L"PAIRS/041_022", L"PAIRS2/02_041_041");
	//zStatProcs::merge_pairs_files(L"PAIRS/042_023", L"PAIRS/042_024", L"PAIRS2/02_042_042");

	//zStatProcs::merge_pairs_files(L"PAIRS/043_025", L"PAIRS/044_026", L"PAIRS2/02_043_044");
	//zStatProcs::merge_pairs_files(L"PAIRS/044_027", L"PAIRS/044_028", L"PAIRS2/02_044_044");
	//zStatProcs::merge_pairs_files(L"PAIRS/045_029", L"PAIRS/046_030", L"PAIRS2/02_045_046");
	//zStatProcs::merge_pairs_files(L"PAIRS/046_031", L"PAIRS/047_032", L"PAIRS2/02_046_047");

	//zStatProcs::merge_pairs_files(L"PAIRS/048_033", L"PAIRS/049_034", L"PAIRS2/02_048_049");
	//zStatProcs::merge_pairs_files(L"PAIRS/049_035", L"PAIRS/050_036", L"PAIRS2/02_049_050");
	//zStatProcs::merge_pairs_files(L"PAIRS/051_037", L"PAIRS/052_038", L"PAIRS2/02_051_052");
	//zStatProcs::merge_pairs_files(L"PAIRS/052_039", L"PAIRS/053_040", L"PAIRS2/02_052_053");

	//zStatProcs::merge_pairs_files(L"PAIRS/054_041", L"PAIRS/054_042", L"PAIRS2/02_054_054");
	//zStatProcs::merge_pairs_files(L"PAIRS/055_043", L"PAIRS/056_044", L"PAIRS2/02_055_056");
	//zStatProcs::merge_pairs_files(L"PAIRS/057_045", L"PAIRS/057_046", L"PAIRS2/02_057_057");
	//zStatProcs::merge_pairs_files(L"PAIRS/058_047", L"PAIRS/059_048", L"PAIRS2/02_058_059");

	//zStatProcs::merge_pairs_files(L"PAIRS/059_049", L"PAIRS/060_050", L"PAIRS2/02_059_060");
	//zStatProcs::merge_pairs_files(L"PAIRS/060_051", L"PAIRS/061_052", L"PAIRS2/02_060_061");
	//zStatProcs::merge_pairs_files(L"PAIRS/062_053", L"PAIRS/063_054", L"PAIRS2/02_062_063");
	//zStatProcs::merge_pairs_files(L"PAIRS/063_055", L"PAIRS/last_056", L"PAIRS2/02_063_0last");

	//zStatProcs::merge_pairs_files(L"PAIRS2/02_000_001", L"PAIRS2/02_003_004", L"PAIRS3/03_000_003");
	//zStatProcs::merge_pairs_files(L"PAIRS2/02_005_007", L"PAIRS2/02_008_010", L"PAIRS3/03_005_008");
	//zStatProcs::merge_pairs_files(L"PAIRS2/02_011_013", L"PAIRS2/02_014_015", L"PAIRS3/03_011_014");
	//zStatProcs::merge_pairs_files(L"PAIRS2/02_016_017", L"PAIRS2/02_017_018", L"PAIRS3/03_016_017");

	//zStatProcs::merge_pairs_files(L"PAIRS2/02_019_019", L"PAIRS2/02_020_020", L"PAIRS3/03_019_020");
	//zStatProcs::merge_pairs_files(L"PAIRS2/02_021_021", L"PAIRS2/02_022_024", L"PAIRS3/03_021_022");
	//zStatProcs::merge_pairs_files(L"PAIRS2/02_024_025", L"PAIRS2/02_026_026", L"PAIRS3/03_024_026");
	//zStatProcs::merge_pairs_files(L"PAIRS2/02_026_027", L"PAIRS2/02_027_028", L"PAIRS3/03_026_027");

	//zStatProcs::merge_pairs_files(L"PAIRS2/02_029_029", L"PAIRS2/02_030_031", L"PAIRS3/03_029_030");
	//zStatProcs::merge_pairs_files(L"PAIRS2/02_032_032", L"PAIRS2/02_033_034", L"PAIRS3/03_032_033");
	//zStatProcs::merge_pairs_files(L"PAIRS2/02_035_036", L"PAIRS2/02_036_037", L"PAIRS3/03_036_036");
	//zStatProcs::merge_pairs_files(L"PAIRS2/02_038_038", L"PAIRS2/02_039_040", L"PAIRS3/03_038_039");

	//zStatProcs::merge_pairs_files(L"PAIRS2/02_041_041", L"PAIRS2/02_042_042", L"PAIRS3/03_041_042");
	//zStatProcs::merge_pairs_files(L"PAIRS2/02_043_044", L"PAIRS2/02_044_044", L"PAIRS3/03_043_044");
	//zStatProcs::merge_pairs_files(L"PAIRS2/02_045_046", L"PAIRS2/02_046_047", L"PAIRS3/03_045_046");
	//zStatProcs::merge_pairs_files(L"PAIRS2/02_048_049", L"PAIRS2/02_049_050", L"PAIRS3/03_048_049");

	//zStatProcs::merge_pairs_files(L"PAIRS2/02_051_052", L"PAIRS2/02_052_053", L"PAIRS3/03_051_052");
	//zStatProcs::merge_pairs_files(L"PAIRS2/02_054_054", L"PAIRS2/02_055_056", L"PAIRS3/03_054_055");
	//zStatProcs::merge_pairs_files(L"PAIRS2/02_057_057", L"PAIRS2/02_058_059", L"PAIRS3/03_057_058");
	//zStatProcs::merge_pairs_files(L"PAIRS2/02_059_060", L"PAIRS2/02_060_061", L"PAIRS3/03_059_060");
	//zStatProcs::merge_pairs_files(L"PAIRS2/02_062_063", L"PAIRS2/02_063_0last", L"PAIRS3/03_062_063");

	//zStatProcs::merge_pairs_files(L"PAIRS3/03_000_003", L"PAIRS3/03_005_008", L"PAIRS4/04_000_005");
	//zStatProcs::merge_pairs_files(L"PAIRS3/03_011_014", L"PAIRS3/03_016_017", L"PAIRS4/04_011_016");
	//zStatProcs::merge_pairs_files(L"PAIRS3/03_019_020", L"PAIRS3/03_021_022", L"PAIRS4/04_019_021");
	//zStatProcs::merge_pairs_files(L"PAIRS3/03_024_026", L"PAIRS3/03_026_027", L"PAIRS4/04_024_026");
	//zStatProcs::merge_pairs_files(L"PAIRS3/03_029_030", L"PAIRS3/03_032_033", L"PAIRS4/04_029_032");
	//zStatProcs::merge_pairs_files(L"PAIRS3/03_036_036", L"PAIRS3/03_038_039", L"PAIRS4/04_036_038");

	//zStatProcs::merge_pairs_files(L"PAIRS3/03_041_042", L"PAIRS3/03_043_044", L"PAIRS4/04_041_043");
	//zStatProcs::merge_pairs_files(L"PAIRS3/03_045_046", L"PAIRS3/03_048_049", L"PAIRS4/04_045_048");
	//zStatProcs::merge_pairs_files(L"PAIRS3/03_051_052", L"PAIRS3/03_054_055", L"PAIRS4/04_051_054");
	//zStatProcs::merge_pairs_files(L"PAIRS3/03_057_058", L"PAIRS3/03_059_060", L"PAIRS4/04_057_059");

	//zStatProcs::merge_pairs_files(L"PAIRS4/04_000_005", L"PAIRS4/04_011_016", L"PAIRS5/05_000_011");
	//zStatProcs::merge_pairs_files(L"PAIRS4/04_019_021", L"PAIRS4/04_024_026", L"PAIRS5/05_019_024");
	//zStatProcs::merge_pairs_files(L"PAIRS4/04_029_032", L"PAIRS4/04_036_038", L"PAIRS5/05_029_036");
	//zStatProcs::merge_pairs_files(L"PAIRS4/04_041_043", L"PAIRS4/04_045_048", L"PAIRS5/05_041_045");
	//zStatProcs::merge_pairs_files(L"PAIRS4/04_051_054", L"PAIRS4/04_057_059", L"PAIRS5/05_051_057");

	//zStatProcs::merge_pairs_files(L"PAIRS5/05_000_011", L"PAIRS5/05_019_024", L"PAIRS6/06_000_019");
	//zStatProcs::merge_pairs_files(L"PAIRS5/05_029_036", L"PAIRS5/05_041_045", L"PAIRS6/06_029_041");
	//zStatProcs::merge_pairs_files(L"PAIRS5/05_051_057", L"PAIRS5/z03_062_063", L"PAIRS6/06_051_062");

	//zStatProcs::merge_pairs_files(L"PAIRS6/06_000_019", L"PAIRS6/06_029_041", L"PAIRS7/07_000_029");
	//zStatProcs::merge_pairs_files(L"PAIRS7/07_000_029", L"PAIRS7/z06_051_062", L"PAIRS8/08_000_051");
}


//------------------------------------------------------------------------------------------
void zParserApp::main_proc()
{
	//_fill_DB();

	//int kk = 70263 / 3000 + 1;
	//for(int ii = 0; ii < kk; ii++)
	//{
	//	int dd = 3000;
	//	size_t first = (size_t)ii * dd;
	//	size_t last = (size_t)(ii+1) * dd - 1;
	//	_read_books(first, last, ii);
	//	_test_load_from_file_bin(ii);
	//}

	//_test_04();
	//output_book("fb2/187151.fb2");
	//test_bases_read_speed();
	//test_wordsDB_read_speed();

	//process_01();

	//output_bad_books();
	//dec_bad_words();
	//output_bad_or_good_words();

	//calc_words_count();
	//output_words_low_books();

	//find_books_rare_words();
	//output_books_with_rare_words();

	//_clean_DB();
	//recalc_words_freqs();

	//_test_fill_pairs_db();
	//merge_pairs();

	//zStatProcs stat_procs;
	//stat_procs.check_pairs_stat(0, 100000);
	//stat_procs.save_to_text_format(L"PAIRS8/08_000_051", 0, 100000, L"PAIRS8_stat/0_100000.stat", L"PAIRS8_stat/0_100000.stat.txt");
	//stat_procs.test_binary_search();
	//stat_procs.save_to_text_format(L"PAIRS8/08_000_051", 0, 10000000, L"PAIRS8_stat/0_10000000.stat", L"PAIRS8_stat/0_10000000.stat.txt");
	//stat_procs.test_output_one_letter_sent();

	//stat_procs.check_pairs_stat(0, 40 * 1000 * 1000);
	//stat_procs.check_pairs_stat(40 * 1000 * 1000, 80 * 1000 * 1000);
	//stat_procs.check_pairs_stat(80 * 1000 * 1000, 120 * 1000 * 1000);
	//stat_procs.check_pairs_stat(120 * 1000 * 1000, 160 * 1000 * 1000);
	//stat_procs.check_pairs_stat(160 * 1000 * 1000, 200 * 1000 * 1000);
	//stat_procs.check_pairs_stat(200 * 1000 * 1000, 240 * 1000 * 1000);
	//stat_procs.check_pairs_stat(240 * 1000 * 1000, 280 * 1000 * 1000);
	//stat_procs.check_pairs_stat(280 * 1000 * 1000, 320 * 1000 * 1000);
	//stat_procs.check_pairs_stat(320 * 1000 * 1000, 370 * 1000 * 1000);
	//stat_procs.check_pairs_stat(370 * 1000 * 1000, 430 * 1000 * 1000);

	//if(0)
	//{
	//	stat_procs.test_find_rare_pairs(L"0_40000000.stat");
	//	stat_procs.test_find_rare_pairs(L"120000000_160000000.stat");
	//	stat_procs.test_find_rare_pairs(L"160000000_200000000.stat");
	//	stat_procs.test_find_rare_pairs(L"200000000_240000000.stat");
	//	stat_procs.test_find_rare_pairs(L"240000000_280000000.stat");
	//	stat_procs.test_find_rare_pairs(L"280000000_320000000.stat");
	//	stat_procs.test_find_rare_pairs(L"320000000_370000000.stat");
	//	stat_procs.test_find_rare_pairs(L"370000000_430000000.stat");
	//	stat_procs.test_find_rare_pairs(L"40000000_80000000.stat");
	//	stat_procs.test_find_rare_pairs(L"80000000_120000000.stat");
	//}

	//stat_procs.output_best_pairs_file();

	//stat_procs.calc_pairs_word3();
	//stat_procs.test_find_rare_pairs(L"words3.cnt");
	//stat_procs.test_best50kk_subarr_len();


	//zStatProcs stat_procs;
	//stat_procs.find_first_pairs();
	//stat_procs.test_find_rare_pairs(L"first_pairs.dat");
	//stat_procs.find_last_words();

	//zGenProcs gen_procs;
	//gen_procs.test_gen_sentences();
	//gen_procs.show_first_pairs_stat();

	//gen_procs().test_performance();
	gen_procs().convert_w3db_to_3bytes();
}

