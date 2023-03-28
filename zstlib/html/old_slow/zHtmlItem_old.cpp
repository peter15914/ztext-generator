#include "stdafx.h"

#include "zHtmlItem_old.h"

#include <zstlib/textutl.h>
#include "zHtmlParser_old.h"	//для find_ignore_const_strs
#include "zHtml_old.h"		//для is_close_tag_warn_except


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace std;
using namespace h_old;


//ВОЗМОЖНО временные функции
namespace zst
{
	inline bool is_equal_(const char *s1, const char *s2)
	{
		return strcmp(s1, s2) == 0;
	}
};


//------------------------------------------------------------------------------------------
zHtmlItem_old::zHtmlItem_old() :
	m_type(HTML_UNKNOWN),
	m_dont_print(false),
	m_flags(0),
	m_close_tag(false),
	m_self_close_tag(false),
	m_parent(0),
	m_str_(0),
	m_tag_(0)
{
}


//------------------------------------------------------------------------------------------
zHtmlItem_old::~zHtmlItem_old()
{
}


//------------------------------------------------------------------------------------------
//возвращает true, если это плохой тэг (типа <script>)
//для таких тэгов мы считаем всё, что между открывающим и закрывающим тэгом, просто текстом
bool zHtmlItem_old::is_bad_tag() const
{
	if(m_type == HTML_TAG)
	{
		if(!strcmp(m_tag_, "script") || !strcmp(m_tag_, "embed") || !strcmp(m_tag_, "form"))
			return true;

		//<style type="text/css">
		if(!strcmp(m_tag_, "style") && has_value("type") && zstr::ToUpper_(get_value("type")) == "TEXT/CSS")
			return true;
	}

	return false;
}


//------------------------------------------------------------------------------------------
//возвращает закрывающий тэг для данного
std::string zHtmlItem_old::get_close_tag() const
{
	rel_assert(!m_close_tag);
	return string("/") + m_tag_;
}


//------------------------------------------------------------------------------------------
//выдает строку для вывода в файл
std::string zHtmlItem_old::get_print_string()
{
	if(m_dont_print)
		return string();

	string ret;

	if(m_type == HTML_COMMENT)
		ret = zstr::fmt("<!--%s-->\n", m_str_);
	else
	if(m_type == HTML_TEXT)
	{
		ret = m_str_;
	}
	else
	if(m_type == HTML_PHP)
	{
		ret = zstr::fmt("<%s>", m_str_);
	}
	else
	if(m_type == HTML_TAG && (m_flags & HF_DOCTYPE))
	{
		ret = zstr::fmt("<%s>", m_str_);
	}
	else
	if(m_type == HTML_TAG)
	{
		//ret = zstr::fmt("<%s>\n", m_str_);
		//ret = zstr::fmt("<%s>", m_str_);

		if(m_close_tag)
			ret = string("</") + m_tag_;
		else
			ret = string("<") + m_tag_;

		for(int i = 0; i < (int)m_values.size(); i++)
		{
			if(!m_values[i].hidden())
			{
				ret += ' ';
				ret += m_values[i].name();
				ret += '=';

				if(m_values[i].quotes())
					ret += '"';

				ret += m_values[i].val();

				if(m_values[i].quotes())
					ret += '"';
			}
		}

		if(m_self_close_tag)
			ret += '/';

		ret += ">";

		//так удобнее
		if(strcmp(m_tag_, "head") == 0)
			ret = "\n" + ret;
	}
	else
		(rel_assert(0));

	return ret;
}


//------------------------------------------------------------------------------------------
void zHtmlItem_old::create_from_string(const std::string &str, HTML_ITEM_TYPE type)
{
	m_type = type;
	m_str_ = m_parent->get_str_pool().AllocString_(str);
	m_tag_ = StringPool::STATIC_EMPTY_STRING;

	//только для тэгов имеет смысл делать разбор
	if(type != HTML_TAG)
		return;

	//
	string buf_norm = str;
	zstr::trim(buf_norm);

	string buf_lower = buf_norm;
	textutl::to_lower_eng(buf_lower, 0);

	if(buf_norm.empty())
		return (rel_assert(0));		//пустой тэг, ошибка

	//
	if(buf_norm[0] == '!')
	{
		if(zstr::is_first("!doctype", buf_lower.c_str()))
			m_flags |= HF_DOCTYPE;
		else
			rel_assert(0);

		return;
	}

	if(buf_lower[0] == '/')
	{
		//закрывающий тэг
		m_close_tag = true;

		int jj = zstr::get_first_non_letter_or_num_tag(buf_lower.c_str(), 1);
		if(jj == -1)
			m_tag_ = m_parent->get_str_pool().AllocString_(buf_lower, 1, buf_lower.length());//buf_lower.substr(1, buf_lower.size());
		else
		{
			m_tag_ = m_parent->get_str_pool().AllocString_(buf_lower, 1, jj-1);//buf_lower.substr(1, jj-1);

			if(!zst::is_equal_(m_tag_, "fb") && buf_lower != "/g:plusone" && buf_lower != "/g:plus")
			{
				//вроде ж у закрывающих ничего не должно быть
				//если нет в списке исключений, то ассертаемся
				if(!m_parent || !m_parent->is_close_tag_warn_except(buf_lower))
					(_ass(0));
			}
			
		}

		return;
	}

	//?php
	if(buf_lower[0] == '?')
	{
		_ass(buf_lower.length() >= 6);
		m_type = HTML_PHP;
		m_tag_ = StringPool::STATIC_EMPTY_STRING;
		m_str_ = m_parent->get_str_pool().AllocString_(buf_norm);

		return;
	}

	//сначала получаем сам тэг
	int jj = zstr::get_first_non_letter_or_num_tag(buf_lower.c_str(), 0);
	if(jj == -1)
	{
		//тут только тэг, свойств нет
		m_tag_ = m_parent->get_str_pool().AllocString_(buf_lower);
		return;
	}

	m_tag_ = m_parent->get_str_pool().AllocString_(buf_lower, 0, jj);//buf_lower.substr(0, jj);
	int sz = (int)buf_lower.size();

	//проверяем, вдруг это самозакрывающий тэг, типа '<br/>'
	for(int i = sz-1; i >= jj; i--)
	{
		if(buf_lower[i] == '/')
		{
			m_self_close_tag = true;

			//обрубим
			buf_norm = buf_norm.substr(0, i);
			zstr::trim(buf_norm);
			buf_lower = buf_norm;
			textutl::to_lower_eng(buf_lower, 0);
			sz = (int)buf_lower.size();

			break;
		}
		if(buf_lower[i] != ' ' && buf_lower[i] != '\t')
			break;
	}


	//достаем все свойства из оставшейся части строки

	int prev = jj+1;
	while(prev < sz)
	{
		int jj2 = zHtmlParser_old::find_ignore_const_strs(buf_lower, "=", prev);
		if(jj2 == -1)
		{
			//проверяем, что остались только пробельные символы
			int i = prev;
			for(; i < sz; i++)
			{
				if(buf_lower[i] != ' ' && buf_lower[i] != '\t')
					break;
			}

			//(было закомментировано, не помню почему), раскомментировал, в след. раз нужно помечать причину
			if(i < sz)
			{
				if(m_self_close_tag && buf_lower[i] == '/')
					;
				else
				{
					string ss = buf_lower.substr(i, sz);
					if(!m_parent || !m_parent->is_no_prop_warn_except(ss))
						_warn("какие-то ошметки: " + ss + "             //from tag: <" + buf_lower + ">");
				}
			}

			break;
		}

		//пропускаем пробелы
		int jj2_buf = jj2+1;
		while(jj2_buf < sz && (buf_lower[jj2_buf] == ' ' || buf_lower[jj2_buf] == '\t'))
			jj2_buf++;

		int jj3 = zHtmlParser_old::find_ignore_const_strs(buf_lower, " ", jj2_buf);
		if(jj3 == -1)
			jj3 = zHtmlParser_old::find_ignore_const_strs(buf_lower, "\t", jj2_buf);

		if(jj3 == -1)
			jj3 = (int)buf_lower.size();	//последний параметр, конец строки


		string param_name = buf_lower.substr(prev, jj2-prev);
		string param_val = buf_norm.substr(jj2+1, jj3-(jj2+1));

		zstr::trim(param_name);
		zstr::trim(param_val);
		if(param_name.empty() || param_val.empty())
			(rel_assert(0));
		else
		{
			zHtmlValue_old vv;
			bool quotes = (param_val.size() >= 2) && (param_val[0] == '"') && (param_val[param_val.size()-1] == '"');
			if(!quotes)
				vv.set_vals(param_name, param_val, quotes);
			else
			{
				param_val = param_val.substr(1, param_val.size() - 2);
				vv.set_vals(param_name, param_val, quotes);
			}

			m_values.push_back(vv);
		}

		prev = jj3;
	}
}


//------------------------------------------------------------------------------------------
bool zHtmlItem_old::has_value(const std::string &value_name) const
{
	for(int i = 0; i < (int)m_values.size(); i++)
	{
		if(m_values[i].name() == value_name)
			return true;
	}
	return false;
}


//------------------------------------------------------------------------------------------
std::string zHtmlItem_old::get_value(const std::string &value_name) const
{
	for(int i = 0; i < (int)m_values.size(); i++)
	{
		if(m_values[i].name() == value_name)
			return m_values[i].val();
	}

	_ass(0);
	return string();
}


//------------------------------------------------------------------------------------------
const char *zHtmlItem_old::get_value_if_has_(const std::string &value_name, const char *dflt_val/* = ""*/) const
{
	for(int i = 0; i < (int)m_values.size(); i++)
	{
		if(m_values[i].name() == value_name)
			return m_values[i].val().c_str();
	}

	return dflt_val;
}


//------------------------------------------------------------------------------------------
bool zHtmlItem_old::has_val(const char *value_name, const char *value_val) const
{
	for(int i = 0; i < (int)m_values.size(); i++)
	{
		if(m_values[i].name() == value_name)
		{
			return m_values[i].val() == value_val;
		}
	}

	return false;
}
