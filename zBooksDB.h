#pragma once


class iWordsDB;


//------------------------------------------------------------------------------------------
class zBookInfo
{
	std::string m_name;
	std::vector<int> m_words;

	int m_snt_added;		//кол-во нормальных предложений
	int m_snt_wrong_words;	//кол-во предложений, недобавленных из-за плохих слов
	int m_snt_wrong_len;	//кол-во предложений, недобавленных из-за короткой длины

public:
	zBookInfo() { m_words.reserve(128); };
	virtual ~zBookInfo() {};

	void set_name(const std::string &name) {m_name = name; };
	const std::string &get_name() { return m_name; }

	void add_word(int word) { m_words.push_back(word); };
	const std::vector<int> &get_words() { return m_words; };

	void set_snt_stat(int snt_added, int snt_wrong_words, int snt_wrong_len)
	{
		m_snt_added = snt_added;
		m_snt_wrong_words = snt_wrong_words;
		m_snt_wrong_len = snt_wrong_len;
	}

	int get_snt_added() { return m_snt_added; };
	int get_snt_wrong_words() { return m_snt_wrong_words; }
	int get_snt_wrong_len() { return m_snt_wrong_len; }

	std::string get_stat_str();

	//процент хороших предложений
	double get_good_percentage();
};


//------------------------------------------------------------------------------------------
class zBooksDB
{
	iWordsDB *m_pwordsDB;

	std::vector<zBookInfo*> m_books;
	std::set<std::string> m_names;		//чтоб ассертаться при добавлении дубля

	//флаг, чтоб считывать только заголовки книг
	bool m_only_headers;

	//поля для чтения не всего файла, а по книге
	std::ifstream m_file;
	zBookInfo *m_cur_book;
	int m_books_cnt;
	int m_cur_book_ind;

public:
	zBooksDB();
	virtual ~zBooksDB();

	void set_wordsDB(iWordsDB *pwordsDB) { m_pwordsDB = pwordsDB; };

	void set_only_headers(bool only_headers) { m_only_headers = only_headers; }

	bool save_to_file(const std::wstring &file_name);
	bool save_to_file(FILE *pFile, zBookInfo *book);

	//в бинарном формате
	bool save_to_file_bin(const std::wstring &file_name);
	bool load_from_file_bin(const std::wstring &file_name);

	/// функции, чтобы не хранить всю базу целиком
	//начало покнижного чтения из базы
	bool begin_load_from_file_bin(const std::wstring &file_name);
	//загрузить след. книгу
	//prev_offset - смещение текущей книги
	bool to_next_book(int *prev_offset = 0);
	//получить последнюю загруженную книгу
	zBookInfo *get_cur_book() { return m_cur_book; };

	//загрузить книгу по смещению
	bool load_book_by_offset(const std::wstring &file_name, int offset);

	//return id
	int add_book(const std::string &name);

	//
	int get_books_cnt() { return (int)m_books.size(); }
	int get_id_by_num(int num) { return num; }
	zBookInfo *get_book_by_id(int id);

	void clear() { _clear_all(); }

private:
	void _clear_all();

	bool _read_book_from_file(std::ifstream &file, zBookInfo *cur_book);
};
