#pragma once

class zCss;

namespace h_old
{

class zHtml_old;

//------------------------------------------------------------------------------------------
class zHtmlParser_old : public boost::noncopyable
{
	std::wstring m_global_bads_file_name;
	std::wstring m_global_text_file_name;

	FILE *m_global_bads_file;
	FILE *m_global_text_file;

public:
	zHtmlParser_old();
	virtual ~zHtmlParser_old();

	//парсит файл в объект html
	bool parse_html_file_old_slow(const wchar_t *file_name, zHtml_old &html);

	//парсит html-строку в объект html
	//bool parse_html_string(const std::string &buf, zHtml_old &html);

	//парсит html-строку в объект html
	//взята с ревизии 1111
	bool parse_html_string_old_slow(const std::string &buf, zHtml_old &html);

	//парсит файл в объект css
	bool parse_css_file(const wchar_t *file_name, zCss& css);

	//сохранить в файл
	//если auto_format, то переформатируем при выводе
	bool save_to_file(const wchar_t *file_name, zHtml_old& html, bool auto_format, const wchar_t *append_file_name);

	//отладочное сохранение в файл (для юнит-тестов)
	bool save_to_file_dbg(const wchar_t *file_name, zHtml_old& html);

	//сохранить в строку
	void save_to_str(zHtml_old& html, std::string &ret_str);

	//
	bool save_texts_to_change(const wchar_t *file_name, zHtml_old &html);

	//сохраняет в файл список оставшихся href'ов
	bool save_https_left(const wchar_t *file_name, zHtml_old &html);

	//находит первое вхождение sub_str в str, начиная с nBeg, исключая подстроки, находящиеся в
	//константных строках (строки в двойных или одинарных кавычках)
	//перенос строки выключает режим константной строки, т.к. это косяк в html, строковые константы
	//не могут быть на несколько строк файла
	static int find_ignore_const_strs(const std::string &str, const std::string &sub_str, int nBeg,
										std::string limits = "\'\'\"\"");

	static int find_ignore_const_strs_(const char *_str, const char *_sub_str, int _nBeg);

	//сохранить в файл
	bool save_css_to_file(const wchar_t *file_name, zCss &css);


	//открыть глобальный файл, в который пишутся все те же данные, что и в локальные hrefs_left_index.txt
	void set_global_bads_file(const wchar_t *file_name, bool text_files);

	//почистить заголовочную часть
	//действуем по принципу "лучше удалить что-то лишнее, чем оставтиь что-то плохое"
	void clean_head(zHtml_old& html);


private:

	//удаляет комментарии
	void _parse_css_cut_comments(std::string &buf);
};

};
