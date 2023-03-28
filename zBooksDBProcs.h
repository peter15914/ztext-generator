#pragma once


//------------------------------------------------------------------------------------------
struct zBookPlace
{
	int m_db_num;
	int m_offset;

	//неосновная копия данных из базы
	int m_snt_added;
	int m_snt_wrong_words;
	int m_snt_wrong_len;
	int m_total_words;

	//кол-во редких и плохих слов
	int m_rare_or_bad_words;

	zBookPlace() :
		m_db_num(INT_MAX),
		m_offset(INT_MAX),
		m_snt_added(0),
		m_snt_wrong_words(0),
		m_snt_wrong_len(0),
		m_total_words(0),
		m_rare_or_bad_words(0)
	{
	}
};


typedef std::map<std::string, zBookPlace> zBookPlaceMap;

//------------------------------------------------------------------------------------------
class zBooksDBProcs
{
	std::wstring m_folder;

	std::vector<std::wstring> m_DB_files;

	//<book name, place>
	zBookPlaceMap m_Data;

	bool m_bad_books_loaded;
	std::set<std::string> m_bad_books;

public:
	zBooksDBProcs();
	virtual ~zBooksDBProcs();

	void create_books_list();
	bool save_books_list(std::wstring file_name);
	bool load_books_list(std::wstring file_name);

	zBookPlace get_book_place(const std::string &name);
	zBookPlace &get_book_place2(const std::string &name);
	std::wstring get_dbfile_name(int db_num) { return m_DB_files[db_num]; };

	const std::vector<std::wstring> &get_DB_files_list() { return m_DB_files; }

	//int get_books_cnt() { return (int)m_Data.size(); };
	//возможно временная функция
	const zBookPlaceMap &get_all_books() { return m_Data; };

	bool is_bad_book(const std::string &book_name);

private:
	//загружает список db-файлов в m_DB_files
	void _load_files_list();

	void _load_bad_books_list();
};
