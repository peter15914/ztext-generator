#pragma once


#include <zstlib/html/zHtml.h>
#include <zstlib/html/zHtmlParser.h>


class zcleaner : public zHtml
{
	zHtmlParser m_parser;
	//глубина бэкапов, иногда ее надо отключать
	int m_backups_depth;

public:
	zcleaner();
	virtual ~zcleaner();

	zHtmlParser &get_parser() { return m_parser; };

	bool parse_html_file(const char *file_name);
	bool save_texts_to_change(const char *file_name);
	bool save_https_left(const char *file_name);
	bool save_to_file(const char *_file_name, const char *_append_file_name);

	//копирует вспомогательные файлы в папку с почищенным html
	void copy_some_files(const char *work_dir, const char *file_name, bool replace);

	void clean_css_file(const wchar_t *file_name, int add_rand_items, int hide_attrs_with_http);
	void clean_css_dir(const wchar_t *_work_dir, int add_rand_items, int hide_attrs_with_http);

	void clean_head();

	//вставить рандомные пустые теги
	void insert_rand_tags();

	void set_backups_depth(int backups_depth) { m_backups_depth = backups_depth; }

protected:
	//выполнить чистку
	//версия, кот. работает через cpp (старая)
	void do_clean();
};

