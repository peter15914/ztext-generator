#include "stdafx.h"

#include "zHtml.h"
#include "zHtmlItem.h"

#include <zstlib/textutl.h>


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;


//------------------------------------------------------------------------------------------
#define PREALLOC_SIZE_ITEMS		4096
#define PREALLOC_SIZE_VALUES	4096


//------------------------------------------------------------------------------------------
void zHtml::constructor_init_params()
{
	m_main_str = 0;

	m_values_.init();
	m_data.init();

	#if PREALLOC_SIZE_ITEMS > 0
		m_data.reserve(PREALLOC_SIZE_ITEMS);
	#endif

	#if PREALLOC_SIZE_VALUES > 0
		m_values_.reserve(PREALLOC_SIZE_VALUES);
	#endif

	m_items_allocator.SetParams(sizeof(zHtmlItem), 1024 * 4);

	m_skip_noindex = false;
	m_were_errors = false;
}


//------------------------------------------------------------------------------------------
zHtml::zHtml()
{
	constructor_init_params();
}


//------------------------------------------------------------------------------------------
zHtml::~zHtml()
{
	clear();
}


//------------------------------------------------------------------------------------------
void zHtml::clear()
{
	m_items_allocator.FreeAll();

	m_data.release();

	free(m_main_str);
	m_main_str = 0;

	m_values_.release();
}


//------------------------------------------------------------------------------------------
char *zHtml::create_main_str(int size)
{
	if(m_main_str)
		return (_ass(0), 0);

	m_main_str = (char*)malloc(sizeof(char) * (size + 1));

	return m_main_str;
}


//------------------------------------------------------------------------------------------
//добавить item
const zHtmlItem &zHtml::add_item(const zsubstr &substr, HTML_ITEM_TYPE type)
{
	//static zHtmlItem _test;
	//return _test;

	zHtmlItem *buf = (zHtmlItem*)m_items_allocator.GetMemory();
	buf->init();

	buf->m_parent = this;
	buf->create_from_string(substr, type);
	m_data.push_back(buf);

	return *buf;
}


//------------------------------------------------------------------------------------------
//вставить item со сдвигом, на место ind
void zHtml::insert_item_slow(const zHtmlItem &item, int ind)
{
	UNREFERENCED_PARAMETER(ind);

	zHtmlItem *buf = (zHtmlItem*)m_items_allocator.GetMemory();
	buf->init();

	*buf = item;
	buf->m_parent = this;

	_ass(0);	//todo
	//m_data_.insert(m_data_.begin() + ind, buf);
}


//------------------------------------------------------------------------------------------
void zHtml::pushback_item_slow(const zHtmlItem &item)
{
	_ass(0);	//todo
}


//------------------------------------------------------------------------------------------
//выставляем всем тегам флаг вывода в файл
void zHtml::set_print_all()
{
	for(int i = 0; i < size(); i++)
	{
		zHtmlItem &itm = item(i);
		itm.set_dont_print(false);

		for(int j = 0; j < itm.get_values_cnt(); j++)
			itm.get_value(j).unset_hidden();
	}
}


//------------------------------------------------------------------------------------------
//не выводить комментарии
//возвращает количество спрятанных
int zHtmlWrk::hide_comments()
{
	int dbg_count = 0;

	for(int i = 0; i < size(); i++)
	{
		zHtmlItem &itm = item(i);
		if(itm.get_type() == HTML_COMMENT)
		{
			itm.set_dont_print(true);
			dbg_count++;
		}
	}

	return dbg_count;
}


//------------------------------------------------------------------------------------------
//не выводить в файл тэг вместе с закрывающим
//возвращает количество спрятанных
int zHtmlWrk::hide_tag(const char *_tag)
{
	int dbg_open = 0;
	int dbg_close = 0;

	for(int i = 0; i < size(); i++)
	{
		zHtmlItem &itm = item(i);
		if(itm.is_tag(_tag))
		{
			itm.set_dont_print(true);
			if(itm.is_close_tag())
				dbg_close++;
			else
				dbg_open++;
		}
	}

	rel_assert(dbg_open == dbg_close || !strcmp(_tag, "meta"));

	return dbg_open + dbg_close;
}


//------------------------------------------------------------------------------------------
//не выводить в файл тэг tag вместе с закрывающим и текстом между ними
//возвращает количество спрятанных
int zHtmlWrk::hide_tag_with_text(const char *_tag)
{
	int dbg_open = 0;
	int dbg_close = 0;

	int opened = 0;
	for(int i = 0; i < size(); i++)
	{
		zHtmlItem &itm = item(i);
		if(itm.is_tag(_tag))
		{
			itm.set_dont_print(true);
			if(itm.is_close_tag())
			{
				dbg_close++;
				opened--;
			}
			else
			{
				dbg_open++;
				opened++;
			}
		}
		else
		if(opened)
			itm.set_dont_print(true);
	}

	rel_assert(dbg_open == dbg_close);
	rel_assert(opened == 0);

	return dbg_open + dbg_close;
}


//------------------------------------------------------------------------------------------
//не выводить данное param у всех тэгов tag
//использует m_skip_noindex
//если !_tag, то на _tag не смотрим
int zHtmlWrk::hide_param_(const char *_tag, const char *_param)
{
	int dbg_cnt = 0;
	bool in_noindex = false;

	for(int i = 0; i < size(); i++)
	{
		zHtmlItem &itm = item(i);

		if(m_skip_noindex)
		{
			if(itm.is_tag("noindex"))
			{
				in_noindex = !itm.is_close_tag();
				continue;
			}

			if(in_noindex)
				continue;
		}

		if(!_tag || itm.is_tag(_tag))
		{
			for(int j = 0; j < itm.get_values_cnt(); j++)
			{
				zHtmlValue &val = itm.get_value(j);
				if(val.m_name_.equal_(_param))
				{
					val.set_hidden();
					dbg_cnt++;
				}
			}
		}
	}

	return dbg_cnt;
}


//------------------------------------------------------------------------------------------
//заменить param у всех тэгов tag на строку new_str
//если new_str содержит "%d", то вставляем порядковый номер вместо него
//использует m_skip_noindex
int zHtmlWrk::replace_tag_param_slow(const char *_tag, const char *_param, const char *_new_str)
{
	UNREFERENCED_PARAMETER(_new_str);
	UNREFERENCED_PARAMETER(_param);
	//bool use_format = (new_str.find("%d") != string.npos);

	bool in_noindex = false;

	int cnt = 0;
	for(int i = 0; i < size(); i++)
	{
		zHtmlItem &itm = item(i);

		if(itm.is_tag("noindex"))
		{
			in_noindex = !itm.is_close_tag();
			continue;
		}

		if(m_skip_noindex && in_noindex)
			continue;

		if(itm.is_tag(_tag))
		{
			for(int j = 0; j < itm.get_values_cnt(); j++)
			{
				_ass(0);	//todo
				/*
				zHtmlValue &val = itm.get_value(j);
				if(values[j].m_name_.equal_(param))
				{
					//не меняем, если там есть <?php
					if(values[j].val().substr(0, 5) == "<?php")
						;
					else
					if(!use_format)
						values[j].set_val(new_str);
					else
					{
						//не используем zstr::fmt(), чтб не было фат.вылетов
						string new_str2 = new_str;
						zstr::replace_once(new_str2, "%d", zstr::itoa(cnt, 10).c_str());
						values[j].set_val(new_str2);
					}
					cnt++;
				}
				*/
			}
		}
	}

	return cnt;
}


//------------------------------------------------------------------------------------------
//удаляет пустые строки и ненужные пробельные символы
//обрабатывает только HTML_TEXT
int zHtmlWrk::remove_blank_lines_and_trim()
{
	bool prev_was_empty = false;
	for(int i = 0; i < size(); i++)
	{
		zHtmlItem &itm = item(i);

		if(itm.get_type() == HTML_TEXT)
		{
			_ass(0);	//TODO:
			string str;// = itm.get_str();

			bool has_new_line = (str.find('\n') != string.npos);
			zstr::trim(str);

			if(!str.empty())
				prev_was_empty = false;
			else
			{
				if(has_new_line && !prev_was_empty)
					str = "\n";
				prev_was_empty = true;
			}
		}
		else
		{
			prev_was_empty = itm.is_dont_print();
		}
	}

	return 0;
}


//не выводить в файл тэг tag
//если hide_text, то вместе с закрывающим и текстом между ними
//только для тэгов, у которых есть параметр param со значением value
//если add_quotes, то добавляем двойные кавычки в начало и конец value
//если sub_str, то достаточно, чтобы value просто содержалось в реальном значении параметра
//возвращает количество спрятанных
int zHtmlWrk::hide_param_value_with_text(const char *_tag, const char *_param, const char *_value, bool add_quotes, bool sub_str,
											bool hide_text)
{
	UNREFERENCED_PARAMETER(sub_str);
	UNREFERENCED_PARAMETER(_param);

	string value_ = _value;
	if(add_quotes)
		value_ = '"' + value_ + '"';

	int dbg_open = 0;
	int dbg_close = 0;

	int opened = 0;
	for(int i = 0; i < size(); i++)
	{
		zHtmlItem &itm = item(i);
		if(!itm.is_tag(_tag))
		{
			//какой-то другой тэг или текст. пропускаем его, если находимся внутри удаляемого тега
			if(opened)
				itm.set_dont_print(true);
		}
		else
		{
			if(itm.is_close_tag() || opened)
			{
				//если (!opened), то этот закрывающий тэг - от какого-то "не нашего" тэга, тогда ничего не делаем
				if(opened > 0)
				{
					itm.set_dont_print(true);
					dbg_close++;
					opened--;
				}
			}
			else
			{
				//bool t = false;	//тэг нам подходит

				_ass(0);	//todo
				/*

				vector<zHtmlValue> & values = itm.get_values();
				for(int j = 0; j < (int)values.size(); j++)
				{
					if(values[j].name() == param)
					{
						if(values[j].val() == value || sub_str && values[j].val().find(value) != string.npos)
						{
							t = true;
							break;
						}
					}
				}

				if(t)
				{
					itm.set_dont_print(true);
					if(!hide_text)
					{
						dbg_open = 1;
						break;
					}
					dbg_open++;
					opened++;
				}

				*/
			}
		}
	}

	rel_assert(dbg_open == dbg_close || !hide_text);
	rel_assert(opened == 0);

	return dbg_open + dbg_close;
}


//------------------------------------------------------------------------------------------
//для удобства, просто вызывает функцию zHtmlParser
//парсит файл в объект html
//bool zHtml::parse_html_file(const wchar_t *file_name)
//{
//	zdebug::Log(boost::format("parsing file %s") % zstr::w_to_s(file_name));
//	zdebug::g_was_error2 = false;
//
//	zHtmlParser html_parser;
//	bool ret = html_parser.parse_html_file(file_name, *this);
//
//	if(zdebug::g_was_error2)
//		m_were_errors = true;
//
//	return ret;
//}


//------------------------------------------------------------------------------------------
//добавить к началу или в конец param у всех тэгов tag строку new_str
//если to_end, то добавляем в конец, иначе - в начало
//если not_add_if_has, то не добавляем add_str, если она уже содержится в value
//использует m_skip_noindex
int zHtmlWrk::add_to_tag_param(const char *_tag, const char *_param, const char *_add_str, bool to_end, bool not_add_if_has)
{
	UNREFERENCED_PARAMETER(to_end);
	UNREFERENCED_PARAMETER(not_add_if_has);
	UNREFERENCED_PARAMETER(_param);
	UNREFERENCED_PARAMETER(_add_str);

	bool in_noindex = false;

	int cnt = 0;
	for(int i = 0; i < size(); i++)
	{
		zHtmlItem &itm = item(i);

		if(itm.is_tag("noindex"))
		{
			in_noindex = !itm.is_close_tag();
			continue;
		}

		if(m_skip_noindex && in_noindex)
			continue;

		if(itm.is_tag(_tag))
		{
			_ass(0);	//todo
			/*
			vector<zHtmlValue> & values = itm.get_values();
			for(int j = 0; j < (int)values.size(); j++)
			{
				if(values[j].name() == param)
				{
					if(not_add_if_has)
					{
						//если not_add_if_has, то не добавляем add_str, если она уже содержится в value
						if(values[j].val().find(add_str) != string.npos)
							continue;
					}

					if(!to_end)
						values[j].set_val(add_str + values[j].val());
					else
						values[j].set_val(values[j].val() + add_str);
					cnt++;
				}
			}
			*/
		}
	}

	return cnt;
}


//------------------------------------------------------------------------------------------
int zHtmlWrk::hide_value_with_substr(const char *_tag, const char *_param, const char *_value)
{
	UNREFERENCED_PARAMETER(_param);
	UNREFERENCED_PARAMETER(_value);

	int cnt = 0;

	for(int i = 0; i < size(); i++)
	{
		zHtmlItem &itm = item(i);
		if(!_tag || itm.is_tag(_tag))
		{
			_ass(0);	//todo
			//vector<zHtmlValue> & values = itm.get_values();
			//for(int j = 0; j < (int)values.size(); j++)
			//{
			//	if(values[j].name() == param)
			//	{
			//		if(values[j].val().find(value) != string.npos)
			//		{
			//			values[j].set_val("");
			//			cnt++;
			//		}
			//	}
			//}
		}
	}

	return cnt;
}


//------------------------------------------------------------------------------------------
//заменяет все текста для href на рандомные текста из file_name
int zHtmlWrk::replace_ahref_titles(const wchar_t *file_name)
{
	FILE *f = _wfopen(file_name, L"rt");
	if(!f)
		return (_ass(0), 0);

	vector<string> data;
	static char buf[10000];
	while(fgets(buf, sizeof(buf), f))
	{
		zstr::remove_eoln(buf);
		if(strlen(buf) > 0)
			data.push_back(buf);
	}
	fclose(f);

	string prev1, prev2;

	int cnt = 0;
	bool in_a = false;
	for(int i = 0; i < size(); i++)
	{
		zHtmlItem &itm = item(i);
		if(itm.is_tag("a"))
			in_a = !itm.is_close_tag();

		if(!in_a)
			continue;

		if(itm.get_type() == HTML_TEXT)
		{
			_ass(0);	//TODO!!!
			string s;// = itm.get_str_();
			zstr::trim(s);
			zstr::ToUpper(s);
			if(!s.empty() && s.find("RSS") == string.npos && s.find("TWIT") == string.npos)
			{
				string new_s;
				int ii = 0;
				for(; ii < 20; ii++)
				{
					new_s = data[rand() % data.size()];
					if(new_s != prev1 && new_s != prev2)
						break;
				}
				_ass(ii < 20);


				prev2 = prev1;
				prev1 = new_s;

				_ass(0);	//TODO!!!
				//itm.create_from_string(zstr::ansi_to_utf8(new_s), HTML_TEXT);
			}
		}
	}

	return cnt;
}

