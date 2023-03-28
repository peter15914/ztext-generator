#pragma once


class zWordsDB;
class zBooksDB;


class zWordsDBFill
{
	size_t m_MinSentLen;	//минимальная длина предложения, которое используем

	std::string m_stat_str;
	std::string m_bad_str;	//m_bad_str - для отладки, туда складываются плохие предложения

	//режим, при котором слова не прибавляются в базу, а удаляются
	//для удаления корявых, ошибочных книг
	bool m_dec_mode;

public:
	zWordsDBFill();
	virtual ~zWordsDBFill();

	void set_MinSentLen(size_t MinSentLen) { m_MinSentLen = MinSentLen; };
	void set_dec_mode(bool dec_mode) {m_dec_mode = dec_mode; }

	void add_words_from_file(zWordsDB *db, const std::wstring &file_name, bool is_fb2);
	void add_book_from_file(zBooksDB *db, zWordsDB *words_db, const std::wstring &file_name, bool is_fb2);

	//строка со статистикой, возвращаемая после работы некоторых функций
	const std::string &get_stat_str() { return m_stat_str; };

	const std::string &get_bad_str() { return m_bad_str; };

private:
	//true, если добавлены в базу
	//words меняется! (для оптимизации)
	bool _add_words(zWordsDB *db, std::vector<std::string> &words);

	//возвращает true, если легальное слово
	bool _to_lower(std::string &str);
};
