#pragma once

namespace h_old
{

class zHtml_old;

//------------------------------------------------------------------------------------------
enum HTML_ITEM_TYPE
{
	HTML_UNKNOWN = 0,
	HTML_COMMENT,
	HTML_TAG,
	HTML_TEXT,
	HTML_PHP
};


//различные флаги
enum
{
	HF_DOCTYPE = 0x1		//<!DOCTYPE html
};



//------------------------------------------------------------------------------------------
class zHtmlValue_old
{
	std::string m_name;
	std::string m_val;
	bool m_hidden;
	bool m_quotes;	//при считывании значения у него были кавычки, но мы их отрубили

public:
	zHtmlValue_old() :
		m_hidden(false),
		m_quotes(false)
	{
	}

	void set_vals(const std::string &name, const std::string &val, bool quotes)
	{
		m_name = name;
		m_val = val;
		m_quotes = quotes;
	}

	bool hidden() { return m_hidden; }
	bool quotes() { return m_quotes; }
	const std::string &name() const { return m_name; };
	const std::string &val() const { return m_val; };

	void set_hidden(bool hidden) { m_hidden = hidden; }
	void set_val(const std::string &val) { m_val = val; }
};


//------------------------------------------------------------------------------------------
class zHtmlItem_old
{
	HTML_ITEM_TYPE m_type;
	const char *m_str_;
	const char *m_tag_;
	std::vector<zHtmlValue_old> m_values;

	bool m_dont_print;
	bool m_close_tag;			//закрывающий тэг
	bool m_self_close_tag;		//самозакрывающий тэг, типа '<br/>'
	int m_flags;				//различные флаги, обычно не очень важные

	zHtml_old *m_parent;

public:
	zHtmlItem_old();
	virtual ~zHtmlItem_old();

	void set_parent(zHtml_old *parent) { m_parent = parent; }

	void create_from_string(const std::string &str, HTML_ITEM_TYPE type);

	//возвращает true, если это плохой тэг (типа <script>)
	//для таких тэгов мы считаем всё, что между открывающим и закрывающим тэгом, просто текстом
	bool is_bad_tag() const;

	//
	inline bool is_close_tag() const { return m_close_tag; };
	inline bool is_close_tag(const char *_tag) const
	{
		return m_close_tag && is_tag(_tag);
	};

	inline bool is_open_tag() const { return !m_close_tag; };
	inline bool is_open_tag(const char *_tag) const
	{
		return !m_close_tag && is_tag(_tag);
	};

	inline const char *get_tag_() const { return m_tag_; };
	inline HTML_ITEM_TYPE get_type() const { return m_type; };
	inline const char *get_str_(){ return m_str_; };
	inline int get_flags() { return m_flags; }

	inline bool is_tag() const
	{
		return (get_type() == HTML_TAG);
	}

	inline bool is_tag(const char *_tag) const
	{
		return (get_type() == HTML_TAG) && !strcmp(m_tag_, _tag);
	}
	inline bool is_tag(const std::string &_tag) const
	{
		return is_tag(_tag.c_str());
	}

	inline bool is_tag_not_close(const char *_tag)
	{
		return !m_close_tag && (get_type() == HTML_TAG) && !strcmp(m_tag_, _tag);
	}

	inline bool is_text()
	{
		return (get_type() == HTML_TEXT);
	}

	//
	std::vector<zHtmlValue_old> &get_values(){ return m_values; };
	bool has_value(const std::string &value_name) const;
	std::string get_value(const std::string &value_name) const;

	const char *get_value_if_has_(const std::string &value_name, const char *dflt_val = "") const;

	bool has_val(const char *value_name, const char *value_val) const;

	//возвращает закрывающий тэг для данного
	std::string get_close_tag() const;

	//
	void set_dont_print(bool dont_print) { m_dont_print = dont_print; }
	bool get_dont_print() { return m_dont_print; }

	//выдает строку для вывода в файл
	std::string get_print_string();
};

};	//namespace h_old
