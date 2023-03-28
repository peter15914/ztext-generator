#pragma once

class zHtmlItem;
struct zHtmlValue;

#include "../allocator.h"

#include "zHtmlCommon.h"

#include <zstlib/zsubstr.h>
#include <zstlib/zvec.h>

//struct zsubstr;

//------------------------------------------------------------------------------------------
class zHtml : public boost::noncopyable
{
	friend class zHtmlItem;	//для скорости, чтоб было меньше вызовов функций в дебаге

protected:
	Allocator m_items_allocator;
	zvec<zHtmlItem*> m_data;

	char *m_main_str;	//строка, в которую считывается файл целиком

	//мы считаем, что у закрывающих тэгов не должно быть атрибутов, но иногда такое бывает
	//в m_close_tag_warn_except храним список исключений
	std::set<std::string> m_close_tag_warn_except;

	//мы считаем, что у каждого свойства есть атрибуты, но иногда такое бывает
	//в m_no_prop_warn_except храним список исключений
	std::set<std::string> m_no_prop_warn_except;

	//если m_skip_noindex, то при чистке пропускаем элементы, которые находятся в <noindex></noindex> (не для всех функций)
	bool m_skip_noindex;

	//были warning'и или assert'ы
	bool m_were_errors;

	//values от zHtmlItem - все хранятся в одном месте
	zvec<zHtmlValue> m_values_;

public:
	__forceinline void constructor_init_params();

	zHtml();
	virtual ~zHtml();

	void clear();

	char *create_main_str(int size);

	//для удобства
	__forceinline int size() { return m_data.size(); }
	__forceinline zHtmlItem &item(int ind) { return *m_data[ind]; }
	__forceinline void reserve(int rsrv_size) {m_data.reserve(rsrv_size); }
	__forceinline bool were_errors() { return m_were_errors; }

	//добавить item
	const zHtmlItem &add_item(const zsubstr &substr, HTML_ITEM_TYPE type);

	//вставить item со сдвигом, на место ind
	void insert_item_slow(const zHtmlItem &item, int ind);

	void pushback_item_slow(const zHtmlItem &item);

	//выставляем всем тегам флаг вывода в файл
	void set_print_all();

	//установить m_skip_noindex
	void set_skip_noindex(bool skip_noindex) { m_skip_noindex = skip_noindex; };

	void add_close_tag_warn_except(std::string str)
	{
		m_close_tag_warn_except.insert(zstr::ToUpper_(str));
	};
	bool is_close_tag_warn_except(const std::string &s)
	{
		//TODO: переделать после того, как сделаю zStr
		std::string s2 = zstr::ToUpper_(s);
		return (m_close_tag_warn_except.find(s2) != m_close_tag_warn_except.end());
	}

	bool is_close_tag_warn_except(zsubstr &s)
	{
		//TODO: переделать после того, как сделаю zStr
		return is_close_tag_warn_except(s.as_string_slow());
	}


	void add_no_prop_warn_except(std::string str)
	{
		m_no_prop_warn_except.insert(str);
	};
	bool is_no_prop_warn_except(zsubstr &s)
	{
		//TODO: переделать после того, как сделаю zStr
		return (m_no_prop_warn_except.find(s.as_string_slow()) != m_no_prop_warn_except.end());
	}
};


//------------------------------------------------------------------------------------------
class zHtmlWrk : zHtml
{
public:
	zHtmlWrk() {};
	virtual ~zHtmlWrk() {};

	//__forceinline std::vector<zHtmlItem*> &get_data_for_swap() { return m_data_; }

	//не выводить комментарии
	//возвращает количество спрятанных
	int hide_comments();

	//не выводить в файл тэг вместе с закрывающим
	//возвращает количество спрятанных
	int hide_tag(const char *_tag);

	//не выводить в файл тэг tag вместе с закрывающим и текстом между ними
	//возвращает количество спрятанных
	int hide_tag_with_text(const char *_tag);

	//не выводить данное param у всех тэгов tag
	//использует m_skip_noindex
	//если !_tag, то на _tag не смотрим
	int hide_param_(const char *_tag, const char *_param);

	//не выводить в файл тэг tag
	//если hide_text, то вместе с закрывающим и текстом между ними
	//только для тэгов, у которых есть параметр param со значением value
	//если add_quotes, то добавляем двойные кавычки в начало и конец value
	//если sub_str, то достаточно, чтобы value просто содержалось в реальном значении параметра
	//возвращает количество спрятанных
	int hide_param_value_with_text(const char *_tag, const char *_param, const char *_value, bool add_quotes, bool sub_str,
										bool hide_text);

	//заменить param у всех тэгов tag на строку new_str
	//если new_str содержит "%d", то вставляем порядковый номер вместо него
	//использует m_skip_noindex
	int replace_tag_param_slow(const char *_tag, const char *_param, const char *_new_str);

	//удаляет пустые строки и ненужные пробельные символы
	//обрабатывает только HTML_TEXT
	int remove_blank_lines_and_trim();

	//добавить к началу или в конец param у всех тэгов tag строку new_str
	//если to_end, то добавляем в конец, иначе - в начало
	//если not_add_if_has, то не добавляем add_str, если она уже содержится в value
	//использует m_skip_noindex
	int add_to_tag_param(const char *_tag, const char *_param, const char *_add_str, bool to_end, bool not_add_if_has);

	//
	//если !_tag, то на _tag не смотрим
	int hide_value_with_substr(const char *_tag, const char *_param, const char *_value);

	//заменяет все текста для href на рандомные текста из file_name
	int replace_ahref_titles(const wchar_t *file_name);

private:
};

