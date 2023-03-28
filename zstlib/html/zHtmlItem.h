#pragma once


#include <zstlib/zsubstr.h>

#include "zHtmlCommon.h"


//------------------------------------------------------------------------------------------
struct zHtmlValue
{
	zsubstr m_name_;
	zsubstr m_val_;

	unsigned char m_flags;

	__forceinline void init()
	{
		#ifdef _DEBUG
			m_name_.init();
			m_val_.init();
			m_flags = 0;
		#else
			memset(this, 0, sizeof(*this));
		#endif
	}

	__forceinline void set_vals(zsubstr &_name, zsubstr &_val)
	{
		m_name_ = _name;
		m_val_ = _val;
	}

	__forceinline void set_quotes()
	{
		m_flags |= HVF_QUOTES;
	}

	__forceinline void unset_quotes()
	{
		m_flags &= ~HVF_QUOTES;
	}

	__forceinline bool is_quotes()
	{
		return (m_flags & HVF_QUOTES) != 0;
	}

	__forceinline void set_hidden()
	{
		m_flags |= HVF_HIDDEN;
	}

	__forceinline void unset_hidden()
	{
		m_flags &= ~HVF_HIDDEN;
	}

	__forceinline bool is_hidden()
	{
		return (m_flags & HVF_HIDDEN) != 0;
	}
};

#include "zHtml.h"

//------------------------------------------------------------------------------------------
class zHtmlItem
{
	friend class zHtmlParser;	//для скорости, чтоб было меньше вызовов функций в дебаге
	friend class zHtml;			//для скорости, чтоб было меньше вызовов функций в дебаге
	friend class zHtmlWrk;		//для скорости, чтоб было меньше вызовов функций в дебаге

	zHtml *m_parent;

	int m_values_ind;			//индекс первого zHtmlValue из родительского массива m_values
	int m_values_cnt;			//количество zHtmlValue из родительского массива m_values

	zsubstr m_str;
	zsubstr m_tag;

	HTML_ITEM_TYPE m_type;
	unsigned char m_flags_;				//флаги

public:
	__forceinline void init()
	{
		#ifdef _DEBUG
			m_type = HTML_UNKNOWN;	//0
			m_flags_ = 0;
			m_parent = 0;
			m_values_ind = 0;
			m_values_cnt = 0;
			m_str.init();
			m_tag.init();
		#else
			memset(this, 0, sizeof(*this));
		#endif
	}

	void create_from_string(const zsubstr &substr, HTML_ITEM_TYPE type);

	__forceinline const zsubstr &get_str() const { return m_str; };
	__forceinline std::string get_str_slow() { return m_str.as_string_slow(); };

	//возвращает true, если это плохой тэг (типа <script>)
	//для таких тэгов мы считаем всё, что между открывающим и закрывающим тэгом, просто текстом
	bool is_bad_tag() const;

	//
	__forceinline bool is_close_tag(const char *_tag) const
	{
		return is_close_tag() && is_tag(_tag);
	};

	__forceinline bool is_open_tag() const { return !is_close_tag(); };
	__forceinline bool is_open_tag(const char *_tag) const
	{
		return is_open_tag() && is_tag(_tag);
	};

	__forceinline const zsubstr &get_tag() const { return m_tag; };
	__forceinline HTML_ITEM_TYPE get_type() const { return m_type; };
	__forceinline unsigned char get_flags_() { return m_flags_; }

	__forceinline bool is_tag() const
	{
		return (m_type == HTML_TAG);
	}

	__forceinline bool is_tag(const char *_tag) const
	{
		return (m_type == HTML_TAG) && m_tag.equal_nc(_tag);
	}

	__forceinline bool is_tag_not_close(const char *_tag)
	{
		return !is_close_tag() && is_tag() && m_tag.equal_nc(_tag);
	}

	__forceinline bool is_text()
	{
		return (m_type == HTML_TEXT);
	}

	/// работа с values

	bool has_value_(const char *value_name) const;
	zsubstr get_value_(const char *value_name) const;
	zsubstr get_value_if_has(const char *value_name) const;

	bool has_val(const char *value_name, const char *value_val) const;

	__forceinline int get_values_cnt() const { return m_values_cnt; }
	__forceinline zHtmlValue &get_value(int ind) const { return m_parent->m_values_[ m_values_ind + ind]; }

	// работа с флагами
	__forceinline void set_dont_print(bool val) { val ? m_flags_ |= HF_DONT_PRINT : m_flags_ &= ~HF_DONT_PRINT; }
	__forceinline void set_close_tag(bool val) { val ? m_flags_ |= HF_CLOSE_TAG : m_flags_ &= ~HF_CLOSE_TAG; }
	__forceinline void set_self_close_tag(bool val) { val ? m_flags_ |= HF_SELF_CLOSE_TAG : m_flags_ &= ~HF_SELF_CLOSE_TAG; }

	__forceinline bool is_dont_print() const { return (m_flags_ & HF_DONT_PRINT) != 0; }
	__forceinline bool is_close_tag() const { return (m_flags_ & HF_CLOSE_TAG) != 0; }
	__forceinline bool is_self_close_tag() const { return (m_flags_ & HF_SELF_CLOSE_TAG) != 0; }

	//выдает строку для вывода в файл
	std::string get_print_string_slow_();

private:
	__forceinline zHtmlValue &_add_value()
	{
		int cur_sz = (int)m_parent->m_values_.size();

		//инициализация values если еще не было
		if(m_values_cnt == 0)
			m_values_ind = cur_sz;

		m_parent->m_values_.resize(cur_sz + 1);
		m_values_cnt++;
		return m_parent->m_values_[cur_sz];
	}

};

