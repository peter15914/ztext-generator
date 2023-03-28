#include "stdafx.h"

#include "zHtmlItem.h"

#include <zstlib/textutl.h>
#include <zstlib/zstr.h>

#include "zHtmlParser.h"	//для find_ignore_const_strs
#include "zHtml.h"			//для is_close_tag_warn_except


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace std;


//------------------------------------------------------------------------------------------
//возвращает true, если это плохой тэг (типа <script>)
//для таких тэгов мы считаем всё, что между открывающим и закрывающим тэгом, просто текстом
bool zHtmlItem::is_bad_tag() const
{
	if(m_type == HTML_TAG)
	{
		//оптимизация
		char c = m_tag.str[0];
		if(c != 's' && c != 'S' && c != 'e' && c != 'E' && c != 'f' && c != 'F')
			return false;

		if(m_tag.equal_nc("script") || m_tag.equal_nc("embed") || m_tag.equal_nc("form"))
			return true;

		//<style type="text/css">
		if(m_tag.equal_nc("style") && has_value_("type") && get_value_("type").equal_nc("TEXT/CSS"))
			return true;
	}

	return false;
}


//------------------------------------------------------------------------------------------
//выдает строку для вывода в файл
std::string zHtmlItem::get_print_string_slow_()
{
	char prev_c = m_str.push_last_zero();
	char prev_c2 = m_tag.push_last_zero();

	string ret;

	if(m_type == HTML_COMMENT)
		ret = zstr::fmt("<!--%s-->\n", m_str.str);
	else
	if(m_type == HTML_TEXT)
	{
		ret = m_str.str;
	}
	else
	if(m_type == HTML_PHP)
	{
		ret = zstr::fmt("<%s>", m_str.str);
	}
	else
	if(m_type == HTML_TAG && (m_flags_ & HF_STRANGE_TAG))
	{
		ret = zstr::fmt("<%s>", m_str.str);
	}
	else
	if(m_type == HTML_TAG)
	{
		//ret = zstr::fmt("<%s>\n", m_str.str);
		//ret = zstr::fmt("<%s>", m_str.str);

		if(is_close_tag())
			ret = string("</") + m_tag.str;
		else
			ret = string("<") + m_tag.str;

		for(int i = 0; i < get_values_cnt(); i++)
		{
			zHtmlValue &val = get_value(i);
			if(!val.is_hidden())
			{
				ret += ' ';
				ret += val.m_name_.as_string_slow();
				ret += '=';

				if(val.is_quotes())
					ret += '"';

				ret += val.m_val_.as_string_slow();

				if(val.is_quotes())
					ret += '"';
			}
		}

		if(is_self_close_tag())
			ret += '/';

		ret += ">";

		//так удобнее
		if(m_tag.equal_nc("head"))
			ret = "\n" + ret;
	}
	else
		(_ass(0));

	m_str.pop_prev_char(prev_c);
	m_tag.pop_prev_char(prev_c2);

	return ret;
}


//------------------------------------------------------------------------------------------
void zHtmlItem::create_from_string(const zsubstr &substr, HTML_ITEM_TYPE type)
{
	m_type = type;
	m_str = substr;
	m_tag.init_empty();

	//только для тэгов имеет смысл делать разбор
	if(type != HTML_TAG)
		return;

	//
	zsubstr _buf_norm = substr;
	_buf_norm.trim();

	if(_buf_norm.empty())
		return (_ass(0));		//пустой тэг, ошибка

	//
	if(_buf_norm.str[0] == '!' || _buf_norm.sz > 2 && _buf_norm.str[0] == '/' && _buf_norm.str[1] == '!')
	{
		if(_buf_norm.is_first_nc_slow("!doctype"))
			m_flags_ |= HF_STRANGE_TAG;
		else
		if(_buf_norm.is_first_nc_slow("![cdata") || _buf_norm.is_first_nc_slow("/![cdata"))
		{
			m_flags_ |= HF_STRANGE_TAG;
		}
		else
		{
			_ass(0);
		}

		return;
	}

	if(_buf_norm.str[0] == '/')
	{
		//закрывающий тэг
		set_close_tag(true);

		int jj = zstr::get_first_non_letter_or_num_tag_(_buf_norm, 1);

		//слишком часто попадается ':' в закрывающих тегах, считаем что это нормально
		//пока что добавляем это в сам тег
		if(jj != -1 && _buf_norm.str[jj] == ':')
		{
			jj = zstr::get_first_non_letter_or_num_tag_(_buf_norm, jj+1);
		}

		if(jj == -1)
		{
			m_tag.str = _buf_norm.str + 1;
			m_tag.sz = _buf_norm.sz - 1;
		}
		else
		{
			m_tag.str = _buf_norm.str + 1;
			m_tag.sz = jj - 1;

			if(!m_tag.equal_nc("fb"))
			{
				//вроде ж у закрывающих ничего не должно быть
				//если нет в списке исключений, то ассертаемся
				if(!m_parent->is_close_tag_warn_except(_buf_norm))
					(_ass(0));
			}
			
		}

		return;
	}

	//?php
	if(_buf_norm.str[0] == '?')
	{
		_ass(_buf_norm.size() >= 6);
		m_type = HTML_PHP;
		m_tag.init_empty();
		m_str = _buf_norm;

		return;
	}

	//сначала получаем сам тэг
	int jj = zstr::get_first_non_letter_or_num_tag_(_buf_norm, 0);
	if(jj == -1)
	{
		//тут только тэг, свойств нет
		m_tag = _buf_norm;
		return;
	}

	m_tag = _buf_norm;
	m_tag.sz = jj;

	int sz = _buf_norm.size();

	//проверяем, вдруг это самозакрывающий тэг, типа '<br/>'
	for(int i = sz-1; i >= jj; i--)
	{
		if(_buf_norm.str[i] == '/')
		{
			set_self_close_tag(true);

			//обрубим
			_buf_norm.sz = i;
			_buf_norm.trim();
			sz = i;

			break;
		}

		//встерилил любой другой символ кроме пробельного - значит не закрывающий тег
		if(_buf_norm.str[i] != ' ' && _buf_norm.str[i] != '\t')
			break;
	}


	/// достаем все свойства из оставшейся части строки

	int prev = jj+1;
	while(prev < sz)
	{
		int jj2 = zHtmlParser::find_ignore_const_strs_(_buf_norm, "=", prev);

		if(jj2 == -1)
		{
			//проверяем, что остались только пробельные символы
			int i = prev;
			for(; i < sz; i++)
			{
				if(_buf_norm.str[i] != ' ' && _buf_norm.str[i] != '\t')
					break;
			}

			//(было закомментировано, не помню почему), раскомментировал, в след. раз нужно помечать причину
			if(i < sz)
			{
				if(is_self_close_tag() && _buf_norm.str[i] == '/')
					;
				else
				{
					zsubstr sub_buf;
					sub_buf.str = _buf_norm.str + i;
					sub_buf.sz = sz - i;

					if(!m_parent->is_no_prop_warn_except(sub_buf))
						_warn("какие-то ошметки: " + sub_buf.as_string_slow() + "             //from tag: <" + _buf_norm.as_string_slow() + ">");
				}
			}

			break;
		}

		//пропускаем пробелы
		int jj2_buf = jj2+1;
		while(jj2_buf < sz && (_buf_norm.str[jj2_buf] == ' ' || _buf_norm.str[jj2_buf] == '\t'))
			jj2_buf++;

		int jj3 = zHtmlParser::find_ignore_const_strs_(_buf_norm, " ", jj2_buf);
		if(jj3 == -1)
			jj3 = zHtmlParser::find_ignore_const_strs_(_buf_norm, "\t", jj2_buf);

		if(jj3 == -1)
			jj3 = _buf_norm.sz;	//последний параметр, конец строки


		zsubstr _param_name = _buf_norm.substr(prev, jj2-prev);
		zsubstr _param_val = _buf_norm.substr(jj2+1, jj3-(jj2+1));

		_param_name.trim();
		_param_val.trim();

		if(_param_name.empty() || _param_val.empty())
			(_ass(0));
		else
		{
			zHtmlValue &vv = _add_value();
			vv.m_flags = 0;
			bool quotes = (_param_val.sz >= 2) && (_param_val.str[0] == '"') && (_param_val.str[_param_val.sz-1] == '"');
			if(!quotes)
				vv.set_vals(_param_name, _param_val);
			else
			{
				_param_val.str++;
				_param_val.sz -= 2;
				vv.set_vals(_param_name, _param_val);
				vv.set_quotes();
			}
		}

		prev = jj3;
	}
}


//------------------------------------------------------------------------------------------
bool zHtmlItem::has_value_(const char *value_name) const
{
	for(int i = 0; i < get_values_cnt(); i++)
	{
		zHtmlValue &val = get_value(i);
		if(val.m_name_.equal_nc(value_name))
			return true;
	}
	return false;
}


//------------------------------------------------------------------------------------------
zsubstr zHtmlItem::get_value_(const char *value_name) const
{
	for(int i = 0; i < get_values_cnt(); i++)
	{
		zHtmlValue &val = get_value(i);
		if(val.m_name_.equal_nc(value_name))
			return val.m_val_;
	}

	_ass(0);
	return zsubstr();
}


//------------------------------------------------------------------------------------------
zsubstr zHtmlItem::get_value_if_has(const char *value_name) const
{
	for(int i = 0; i < get_values_cnt(); i++)
	{
		zHtmlValue &val = get_value(i);
		if(val.m_name_.equal_nc(value_name))
			return val.m_val_;
	}

	zsubstr ret;
	ret.init_empty();
	return ret;
}


//------------------------------------------------------------------------------------------
bool zHtmlItem::has_val(const char *value_name, const char *value_val) const
{
	for(int i = 0; i < get_values_cnt(); i++)
	{
		zHtmlValue &val = get_value(i);
		if(val.m_name_.equal_(value_name))
			return val.m_val_.equal_(value_val);
	}

	return false;
}
