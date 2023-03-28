#pragma once


class zTextParser
{
	std::ifstream m_file;

	//в режиме m_isFb2 вырезаются и учитываются тэги
	bool m_isFb2;

	//режим utf8 включается автоматически и только для Fb2
	bool m_bUTF8;

	//открыт какой-то тэг
	bool m_bTagOpened;

	//встречался тэг <section> и еще не было </section>
	bool m_bSectionOpened;

	//открыт тэг <?
	bool m_bXmlTagOpened;

	//
	std::string m_Buf;
	size_t m_iCurInd;

	//чисто отладочное поле - кодировка из хедера файла
	int m_iEncoding;

public:
	zTextParser();
	virtual ~zTextParser();

	bool open(const std::wstring &file_name, bool is_fb2);

	enum LEX_TYPE
	{
		LT_NONE = 0,
		LT_WORD,
		LT_SENTENCE_END,
		LT_FILE_END
	};

	//множественные LT_SENTENCE_END превращает в один
	LEX_TYPE get_next_word(std::string &ret_str);

	//тестовый вывод отпарсенных данных
	void test_write_to_file(const std::wstring &file_name);

	//для вывода отладочной информации
	bool is_utf8() { return m_bUTF8; }

	int get_encoding() { return m_iEncoding; }

private:
	LEX_TYPE m_PrevRet;

	void _init_after_open();
	LEX_TYPE _get_next_word(std::string &ret_str);

};
