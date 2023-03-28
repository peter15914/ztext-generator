#include "stdafx.h"

#include "zBooksDBProcs.h"

#include "zBooksDB.h"


//------------------------------------------------------------------------------------------
zBooksDBProcs::zBooksDBProcs() :
	m_bad_books_loaded(false)
{
	m_folder = L"BOOKS_DB";
	_load_files_list();
}


//------------------------------------------------------------------------------------------
zBooksDBProcs::~zBooksDBProcs()
{
}


//------------------------------------------------------------------------------------------
void zBooksDBProcs::create_books_list()
{
	zBooksDB booksDB;
	booksDB.set_only_headers(true);

	for(size_t i = 0; i < m_DB_files.size(); i++)
	{
		if(booksDB.begin_load_from_file_bin(m_DB_files[i]))
		{
			int offset = 0;
			while(booksDB.to_next_book(&offset))
			{
				zBookInfo *book = booksDB.get_cur_book();

				zBookPlace zz;
				zz.m_db_num = i;
				zz.m_offset = offset;
				zz.m_snt_added = book->get_snt_added();
				zz.m_snt_wrong_words = book->get_snt_wrong_words();
				zz.m_snt_wrong_len = book->get_snt_wrong_len();

				m_Data[book->get_name()] = zz;
			}
		}
		zdebug::log()->Log(zstr::fmt("%d/%d finished\n", (int)i, (int)m_DB_files.size()));
	}
}


//------------------------------------------------------------------------------------------
bool zBooksDBProcs::save_books_list(std::wstring file_name)
{
	file_name = m_folder + L"/" + file_name;

	FILE *pFile = _wfopen(file_name.c_str(), L"w");
	if(!pFile)
		return (rel_assert(0), false);

	fprintf(pFile, "%d\n", (int)m_Data.size());
	for(zBookPlaceMap::iterator it = m_Data.begin(); it != m_Data.end(); it++)
	{
		fputs(it->first.c_str(), pFile);
		fprintf(pFile, "\n%d\n", it->second.m_db_num);
		fprintf(pFile, "%d\n", it->second.m_offset);
		fprintf(pFile, "%d\n", it->second.m_snt_added);
		fprintf(pFile, "%d\n", it->second.m_snt_wrong_words);
		fprintf(pFile, "%d\n", it->second.m_snt_wrong_len);
		fprintf(pFile, "%d\n", it->second.m_total_words);
		fprintf(pFile, "%d\n", it->second.m_rare_or_bad_words);
	}

	fclose(pFile);
	return true;
}


//------------------------------------------------------------------------------------------
bool zBooksDBProcs::load_books_list(std::wstring file_name)
{
	m_Data.clear();

	file_name = m_folder + L"/" + file_name;

	FILE *pFile = _wfopen(file_name.c_str(), L"r");
	if(!pFile)
		return (rel_assert(0), false);

	int size = 0;
	fscanf(pFile, "%d", &size);

	zBookPlace zz;
	for(int i = 0; i < size; i++)
	{
		static char s[1000];
		s[0] = 0;
		int db_num = 0;
		int offset = 0;

		fscanf(pFile, "%s", s);
		fscanf(pFile, "%d", &zz.m_db_num);
		fscanf(pFile, "%d", &zz.m_offset);
		fscanf(pFile, "%d", &zz.m_snt_added);
		fscanf(pFile, "%d", &zz.m_snt_wrong_words);
		fscanf(pFile, "%d", &zz.m_snt_wrong_len);
		fscanf(pFile, "%d", &zz.m_total_words);
		fscanf(pFile, "%d", &zz.m_rare_or_bad_words);

		m_Data[s] = zz;
	}

	fclose(pFile);
	return true;
}


//------------------------------------------------------------------------------------------
//загружает список db-файлов в m_DB_files
void zBooksDBProcs::_load_files_list()
{
	m_DB_files.clear();

	std::wstring name = m_folder + L"/_all_bases";

	std::ifstream files_list(name.c_str());

	std::string s;
	while(files_list)
	{
		std::getline(files_list, s);
		if(!files_list)
			break;

		if(!s.empty())
			m_DB_files.push_back(m_folder + L"/" + zstr::s_to_w(s));
	}
}


//------------------------------------------------------------------------------------------
zBookPlace zBooksDBProcs::get_book_place(const std::string &name)
{
	zBookPlaceMap::iterator i = m_Data.find(name);
	if(i != m_Data.end())
		return i->second;

	return zBookPlace();
}


//------------------------------------------------------------------------------------------
zBookPlace &zBooksDBProcs::get_book_place2(const std::string &name)
{
	zBookPlaceMap::iterator i = m_Data.find(name);
	if(i != m_Data.end())
		return i->second;

	static zBookPlace z;
	return z;
}


//------------------------------------------------------------------------------------------
bool zBooksDBProcs::is_bad_book(const std::string &book_name)
{
	if(!m_bad_books_loaded)
		_load_bad_books_list();

	return m_bad_books.find(book_name) != m_bad_books.end();
}


//------------------------------------------------------------------------------------------
void zBooksDBProcs::_load_bad_books_list()
{
	if(m_bad_books_loaded)
		return;
	m_bad_books_loaded = true;

	std::wstring name = m_folder + L"/_bad_books.txt";
	std::ifstream files_list(name.c_str());

	std::string s;
	while(files_list)
	{
		std::getline(files_list, s);
		if(!files_list)
			break;

		if(!s.empty())
			m_bad_books.insert(s);
	}

}
