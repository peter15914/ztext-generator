#include "stdafx.h"

#include "iWordsDB.h"

#include "zBooksDB.h"


//------------------------------------------------------------------------------------------
std::string zBookInfo::get_stat_str()
{
	std::string ret = m_name;
	ret += ". ";

	ret += zstr::fmt("added=%d. ", m_snt_added);
	ret += zstr::fmt("wrong_words=%d. ", m_snt_wrong_words);
	ret += zstr::fmt("wrong_len=%d. ", m_snt_wrong_len);

	return ret;
}


//------------------------------------------------------------------------------------------
//процент хороших предложений
double zBookInfo::get_good_percentage()
{
	int cnt = m_snt_wrong_words + m_snt_added;
	if(!cnt)
		return (rel_assert(0), 100.0);

	return double(m_snt_added) / double(cnt);
}


//------------------------------------------------------------------------------------------
zBooksDB::zBooksDB() :
	m_pwordsDB(0),
	m_cur_book(0),
	m_books_cnt(0),
	m_cur_book_ind(0),
	m_only_headers(false)
{
}


//------------------------------------------------------------------------------------------
zBooksDB::~zBooksDB()
{
	_clear_all();
	if(m_file.is_open())
		m_file.close();
}


//------------------------------------------------------------------------------------------
void zBooksDB::_clear_all()
{
	for(size_t i = 0; i < m_books.size(); i++)
		delete m_books[i];
	m_books.clear();

	delete m_cur_book;
	m_cur_book = 0;
}


//------------------------------------------------------------------------------------------
int zBooksDB::add_book(const std::string &name)
{
	zBookInfo *book = new zBookInfo();
	m_books.push_back(book);

	if(!name.empty())
	{
		if(m_names.find(name) != m_names.end())
			rel_assert(0);

		m_names.insert(name);
		book->set_name(name);
	}

	return (int)m_books.size()-1;
}


//------------------------------------------------------------------------------------------
zBookInfo *zBooksDB::get_book_by_id(int id)
{
	return m_books[id];
}


//------------------------------------------------------------------------------------------
bool zBooksDB::save_to_file(const std::wstring &file_name)
{
	//if(!m_pwordsDB)
	//	return (rel_assert(0), false);

	zutl::MakeBackupsEx(file_name, 10, L"_bak", 5);

	FILE *pFile = _wfopen(file_name.c_str(), L"w");
	if(!pFile)
		return (rel_assert(0), false);

	//cnt
	fprintf(pFile, "%d\n", (int)m_books.size());

	//data
	for(size_t i = 0; i < m_books.size(); i++)
		save_to_file(pFile, m_books[i]);

	fclose(pFile);
	return true;
}


//------------------------------------------------------------------------------------------
bool zBooksDB::save_to_file(FILE *pFile, zBookInfo *book)
{
	fprintf(pFile, ";---%s", book->get_name().c_str());
	fprintf(pFile, "(%d, %d, %d)\n", book->get_snt_added(), book->get_snt_wrong_words(), book->get_snt_wrong_len());

	const std::vector<int> &words = book->get_words();

	//cnt
	fprintf(pFile, "%d\n", (int)words.size());
	for(size_t j = 0; j < words.size(); j++)
	{
		int word_id = words[j];

		//обычная версия сохранения
		if(0)
			fprintf(pFile, "%d\n", word_id);
		else
		{
			//отладочная версия сохранения
			if(word_id == -1 || !m_pwordsDB)
				fprintf(pFile, ".\n");
			else
			{
				const char *pStr = 0;
				if(m_pwordsDB->is_valid_id(word_id))
					pStr = m_pwordsDB->get_str_by_id(word_id);
				else
					(rel_assert(0));

				if(pStr)
					fprintf(pFile, "%d(%s)\n", word_id, pStr);
				else
					fprintf(pFile, "%d(-)\n", word_id);
			}
		}
	}

	return true;
}


//------------------------------------------------------------------------------------------
//в бинарном формате
bool zBooksDB::save_to_file_bin(const std::wstring &file_name)
{
	zutl::MakeBackupsEx(file_name, 10, L"_bak", 5);
	std::ofstream file(file_name.c_str(), std::ios::binary);
	if(!file)
		return (rel_assert(0), false);

	//буферы для записи
	int buf;
	char *pIntBuf = (char*)(&buf);
	unsigned int uintBuf;
	char *pUintBuf = (char*)(&uintBuf);
	unsigned short ushrtBuf;
	char *pUshrtBuf = (char*)(&ushrtBuf);
	unsigned char ucharBuf;
	char *pUcharBuf = (char*)(&ucharBuf);

	//cnt
	buf = (int)m_books.size();
	file.write(pIntBuf, sizeof(int));

	//data
	for(size_t i = 0; i < m_books.size(); i++)
	{
		zBookInfo *book = m_books[i];

		//длина названия книги
		buf = (int)book->get_name().size();
		file.write(pIntBuf, sizeof(int));
		//название книги
		file.write(book->get_name().c_str(), buf);

		//snt_added
		buf = book->get_snt_added();
		file.write(pIntBuf, sizeof(int));
		//snt_wrong_words
		buf = book->get_snt_wrong_words();
		file.write(pIntBuf, sizeof(int));
		//snt_wrong_len
		buf = book->get_snt_wrong_len();
		file.write(pIntBuf, sizeof(int));

		//
		const std::vector<int> &words = book->get_words();

		//буфер, кот. потом запишем в файл
		std::vector<unsigned char> file_buf;
		file_buf.reserve(words.size() * 3);

		//каждое слово
		int w;
		unsigned int uw;
		bool isSentEnd = false;
		for(size_t j = 0; j < words.size(); j++)
		{
			//первый бит обозначает, сколько байтов занимает слово: 0 - 2 байта, 1 - 3 байта
			//второй бит обозначает, стоит ли перед словом точка
			//0x3fff означает точку в конце книги (со вторым битом)
			w = words[j];
			if(w == BI_SENT_END)
				isSentEnd = true;
			else
			if(w < 0x3fff)	//if(w <= 0x3fff)
			{
				ushrtBuf = (unsigned short)w;
				if(isSentEnd)
				{
					ushrtBuf |= 0x4000;
					isSentEnd = false;
				}

				file_buf.push_back((unsigned char) (ushrtBuf >> 8));
				file_buf.push_back((unsigned char) (ushrtBuf & 0xff));
			}
			else
			{
				uw = (unsigned int)w;

				ucharBuf = (unsigned char) (uw >> 16);
				ushrtBuf = (unsigned short) (uw & 0xffff);

				ucharBuf |= 0x80;	//первый бит обозначает, сколько байтов занимает слово: 1 - 3 байта
				if(isSentEnd)
				{
					ucharBuf |= 0x40;
					isSentEnd = false;
				}
				file_buf.push_back(ucharBuf);
				file_buf.push_back((unsigned char) (ushrtBuf >> 8));
				file_buf.push_back((unsigned char) (ushrtBuf & 0xff));
			}
		}

		//0x3fff означает точку в конце книги (со вторым битом)
		if(isSentEnd)
		{
			ushrtBuf = 0x3fff | 0x4000;

			file_buf.push_back((unsigned char) (ushrtBuf >> 8));
			file_buf.push_back((unsigned char) (ushrtBuf & 0xff));
		}

		//кол-во слов в книге
		buf = (int)file_buf.size();
		file.write(pIntBuf, sizeof(int));
		if(!file_buf.empty())
		{
			file.write((char*)(&file_buf[0]), file_buf.size());
		}
	}

	file.close();
	return true;
}


//------------------------------------------------------------------------------------------
bool zBooksDB::load_from_file_bin(const std::wstring &file_name)
{
	std::ifstream file(file_name.c_str(), std::ios::binary);
	if(!file)
		return (rel_assert(0), false);

	if(!m_books.empty())
		return (rel_assert(0), false);

	//буферы для чтения
	int buf;
	char *pIntBuf = (char*)(&buf);

	//cnt
	file.read(pIntBuf, sizeof(int));
	int cnt = buf;

	//data
	for(int i = 0; i < cnt; i++)
	{
		int book_id = add_book("");
		zBookInfo *book = get_book_by_id(book_id);
		_read_book_from_file(file, book);
	}

	return true;
}


//------------------------------------------------------------------------------------------
bool zBooksDB::_read_book_from_file(std::ifstream &file, zBookInfo *book)
{
	char cbuf[4096];

	//буферы для чтения
	int buf;
	char *pIntBuf = (char*)(&buf);
	unsigned int uintBuf;
	char *pUintBuf = (char*)(&uintBuf);
	unsigned short ushrtBuf;
	char *pUshrtBuf = (char*)(&ushrtBuf);
	unsigned char ucharBuf;
	char *pUcharBuf = (char*)(&ucharBuf);
	unsigned char ucharBuf2;
	char *pUcharBuf2 = (char*)(&ucharBuf2);

	//длина названия книги
	file.read(pIntBuf, sizeof(int));
	int name_len = buf;

	if(name_len >= _countof(cbuf))
		return (rel_assert(0), false);

	//название книги
	file.read(cbuf, name_len);
	cbuf[name_len] = 0;

	book->set_name(cbuf);

	//snt_added
	//snt_wrong_words
	//snt_wrong_len
	int x1, x2, x3;
	file.read((char*)(&x1), sizeof(int));
	file.read((char*)(&x2), sizeof(int));
	file.read((char*)(&x3), sizeof(int));
	book->set_snt_stat(x1, x2, x3);

	//
	//const std::vector<int> &words = book->get_words();

	//длина буфера, записанная в файл
	file.read(pIntBuf, sizeof(int));
	int book_file_len = buf;
	if(!book_file_len)
		return true;

	//если режим чтения только заголовков, то перемещаем указатель в файле и уходим
	if(m_only_headers)
	{
		file.seekg(book_file_len, std::ios::cur);
		return true;
	}

	//буфер, кот. считаем из файла
	std::vector<unsigned char> file_buf(book_file_len, 0);
	file.read((char*)(&file_buf[0]), book_file_len);

	bool ret = true;
	//каждое слово
	for(int read = 0; read < book_file_len; )
	{
		//первый бит обозначает, сколько байтов занимает слово: 0 - 2 байта, 1 - 3 байта
		//второй бит обозначает, стоит ли перед словом точка
		ucharBuf = file_buf[read];
		read++;
		if(read >= book_file_len)
		{
			rel_assert(0);
			ret = false;
			break;
		}

		//второй бит обозначает, стоит ли перед словом точка
		if(ucharBuf & 0x40)
		{
			book->add_word(BI_SENT_END);
			ucharBuf &= ~0x40;
		}

		//первый бит обозначает, сколько байтов занимает слово: 0 - 2 байта, 1 - 3 байта
		if(ucharBuf & 0x80)
		{
			//3 байта
			ucharBuf &= ~0x80;
			unsigned int uw = (ucharBuf << 16);

			ucharBuf = file_buf[read];
			read++;
			if(read >= book_file_len)
			{
				rel_assert(0);
				ret = false;
				break;
			}

			ucharBuf2 = file_buf[read];
			read++;

			uw = uw | (ucharBuf << 8) | ucharBuf2;
			book->add_word((int)uw);
		}
		else
		{
			//2 байта
			ucharBuf2 = file_buf[read];
			read++;

			if(ucharBuf == 0x3f && ucharBuf2 == 0xff)
			{
				rel_assert(read == book_file_len);	//0x3fff может быть только в конце книги
			}
			else
			{
				unsigned int uw = (ucharBuf << 8) | ucharBuf2;
				book->add_word((int)uw);
			}
		}
	}

	return ret;
}


//------------------------------------------------------------------------------------------
//начало покнижного чтения из базы
bool zBooksDB::begin_load_from_file_bin(const std::wstring &file_name)
{
	delete m_cur_book;
	m_cur_book = 0;

	if(m_file.is_open())
		m_file.close();

	m_file.open(file_name.c_str(), std::ios::binary);
	if(!m_file)
		return (rel_assert(0), false);

	//буферы для чтения
	int buf;
	char *pIntBuf = (char*)(&buf);

	//cnt
	m_file.read(pIntBuf, sizeof(int));
	m_books_cnt = buf;

	m_cur_book_ind = 0;
	return true;
}


//------------------------------------------------------------------------------------------
//загрузить след. книгу
bool zBooksDB::to_next_book(int *prev_offset/* = 0*/)
{
	delete m_cur_book;
	m_cur_book = 0;

	if(prev_offset)
		*prev_offset = (int)m_file.tellg();

	if(++m_cur_book_ind > m_books_cnt)
		return false;

	m_cur_book = new zBookInfo();

	return _read_book_from_file(m_file, m_cur_book);
}


//------------------------------------------------------------------------------------------
//загрузить книгу по смещению
bool zBooksDB::load_book_by_offset(const std::wstring &file_name, int offset)
{
	delete m_cur_book;
	m_cur_book = 0;

	if(m_file.is_open())
		m_file.close();

	m_file.open(file_name.c_str(), std::ios::binary);
	if(!m_file)
		return (rel_assert(0), false);

	m_file.seekg(offset, std::ios::beg);

	m_cur_book = new zBookInfo();
	return _read_book_from_file(m_file, m_cur_book);
}

