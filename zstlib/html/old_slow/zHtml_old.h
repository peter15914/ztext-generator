#pragma once

#include "zHtmlItem_old.h"

#include "../../allocator.h"

namespace h_old
{


//------------------------------------------------------------------------------------------
class zHtml_old : public boost::noncopyable
{

protected:
	Allocator m_items_allocator;
	std::vector<zHtmlItem_old*> m_data_;

	StringPool m_str_pool;

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

public:
	inline void constructor_init_params();

	zHtml_old();

	virtual ~zHtml_old();

	//std::vector<zHtmlItem> &get_data() { return m_data; }

	inline StringPool &get_str_pool() { return m_str_pool; };

	char *create_main_str(int size);

	//для удобства
	int size() { return (int)m_data_.size(); }
	inline zHtmlItem_old &item(int ind) { return *m_data_[ind]; }

	void clear();

	void reserve(int rsrv_size) {m_data_.reserve(rsrv_size); }

	//std::vector<zHtmlItem_old*> &get_data_for_swap() { return m_data_; }

	bool were_errors() { return m_were_errors; }

	//добавить item
	const zHtmlItem_old &add_item(const std::string &str, HTML_ITEM_TYPE type);
	const zHtmlItem_old &add_item(const zHtmlItem_old &item);

	//вставить item со сдвигом, на место ind
	void insert_item(const zHtmlItem_old &item, int ind);

	//выставляем всем тегам флаг вывода в файл
	void set_print_all();

	//установить m_skip_noindex
	void set_skip_noindex(bool skip_noindex) { m_skip_noindex = skip_noindex; };

	//не выводить комментарии
	//возвращает количество спрятанных
	int hide_comments();

	//не выводить в файл тэг вместе с закрывающим
	//возвращает количество спрятанных
	int hide_tag(std::string tag);

	//не выводить в файл тэг tag вместе с закрывающим и текстом между ними
	//возвращает количество спрятанных
	int hide_tag_with_text(std::string tag);

	//не выводить данное param у всех тэгов tag
	int hide_param(std::string tag, std::string param);

	//не выводить в файл тэг tag
	//если hide_text, то вместе с закрывающим и текстом между ними
	//только для тэгов, у которых есть параметр param со значением value
	//если add_quotes, то добавляем двойные кавычки в начало и конец value
	//если sub_str, то достаточно, чтобы value просто содержалось в реальном значении параметра
	//возвращает количество спрятанных
	int hide_param_value_with_text(std::string tag, std::string param, std::string value, bool add_quotes, bool sub_str,
										bool hide_text);

	//заменить param у всех тэгов tag на строку new_str
	//если new_str содержит "%d", то вставляем порядковый номер вместо него
	//использует m_skip_noindex
	int replace_tag_param(std::string tag, std::string param, std::string new_str);

	//удаляет пустые строки и ненужные пробельные символы
	//обрабатывает только HTML_TEXT
	int remove_blank_lines_and_trim();

	//добавить к началу или в конец param у всех тэгов tag строку new_str
	//если to_end, то добавляем в конец, иначе - в начало
	//если not_add_if_has, то не добавляем add_str, если она уже содержится в value
	//использует m_skip_noindex
	int add_to_tag_param(std::string tag, std::string param, std::string add_str, bool to_end, bool not_add_if_has);

	//
	int hide_value_with_substr(std::string tag, std::string param, std::string value);

	//заменяет все текста для href на рандомные текста из file_name
	int replace_ahref_titles(const wchar_t *file_name);

	void add_close_tag_warn_except(std::string str)
	{
		m_close_tag_warn_except.insert(str);
	};
	bool is_close_tag_warn_except(const std::string &s)
	{
		return (m_close_tag_warn_except.find(s) != m_close_tag_warn_except.end());
	}

	void add_no_prop_warn_except(std::string str)
	{
		m_no_prop_warn_except.insert(str);
	};
	bool is_no_prop_warn_except(const std::string &s)
	{
		return (m_no_prop_warn_except.find(s) != m_no_prop_warn_except.end());
	}

private:
};

};	//namespace h_old
