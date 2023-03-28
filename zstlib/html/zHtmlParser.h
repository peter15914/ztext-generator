#pragma once

class zHtml;
class zCss;

//------------------------------------------------------------------------------------------
class zHtmlParser : public boost::noncopyable
{
	std::wstring m_global_bads_file_name;
	std::wstring m_global_text_file_name;

	FILE *m_global_bads_file;
	FILE *m_global_text_file;

public:
	zHtmlParser();
	virtual ~zHtmlParser();

	//парсит файл в объект html
	bool parse_html_file(const wchar_t *file_name, zHtml &html);

	//парсит html-строку в объект html
	bool parse_html_string_(char *buf, int size, zHtml &html, bool create_copy);
	bool parse_html_string_slow(std::string buf, zHtml &html);

	//парсит файл в объект css
	bool parse_css_file(const wchar_t *file_name, zCss& css);

	//сохранить в файл
	//если auto_format, то переформатируем при выводе
	bool save_to_file(const wchar_t *file_name, zHtml& html, bool auto_format, const wchar_t *append_file_name);

	//отладочное сохранение в файл (для юнит-тестов)
	bool save_to_file_dbg(const wchar_t *file_name, zHtml& html);

	//сохранить в строку
	void save_to_str(zHtml& html, std::string &ret_str);

	//
	bool save_texts_to_change(const wchar_t *file_name, zHtml &html);

	//сохраняет в файл список оставшихся href'ов
	bool save_https_left(const wchar_t *file_name, zHtml &html);

	//находит первое вхождение sub_str в str, начиная с nBeg, исключая подстроки, находящиеся в
	//константных строках (строки в двойных или одинарных кавычках)
	//перенос строки выключает режим константной строки, т.к. это косяк в html, строковые константы
	//не могут быть на несколько строк файла
	static int find_ignore_const_strs_slow(const std::string &str, const std::string &sub_str, int nBeg,
										std::string limits = "\'\'\"\"");

	static int find_ignore_const_strs_(const char *_str, const char *_sub_str, int _nBeg);
	static int find_ignore_const_strs_(zsubstr _str, const char *_sub_str, int _nBeg);

	//сохранить в файл
	bool save_css_to_file(const wchar_t *file_name, zCss &css);


	//открыть глобальный файл, в который пишутся все те же данные, что и в локальные hrefs_left_index.txt
	void set_global_bads_file(const wchar_t *file_name, bool text_files);

	//почистить заголовочную часть
	//действуем по принципу "лучше удалить что-то лишнее, чем оставтиь что-то плохое"
	void clean_head(zHtml& html);


private:
	//парсит html-строку в объект html
	bool _parse_html_string(char *data, zHtml &html, int size);

	//удаляет комментарии
	void _parse_css_cut_comments(std::string &buf);
};
