#include "stdafx.h"

#include "zHtml_old.h"

#include <zstlib/textutl.h>


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;
using namespace h_old;


//------------------------------------------------------------------------------------------
#define PREALLOC_SIZE 1024


//------------------------------------------------------------------------------------------
void zHtml_old::constructor_init_params()
{
	m_main_str = 0;

	#if PREALLOC_SIZE > 0
		m_data_.reserve(PREALLOC_SIZE);
	#endif

	m_items_allocator.SetParams(sizeof(zHtmlItem_old), 1024 * 4);

	m_skip_noindex = false;
	m_were_errors = false;
}


//------------------------------------------------------------------------------------------
zHtml_old::zHtml_old()
{
	constructor_init_params();
}


//------------------------------------------------------------------------------------------
zHtml_old::~zHtml_old()
{
	clear();
}


//------------------------------------------------------------------------------------------
void zHtml_old::clear()
{
	//уничтожаем объекты из m_data_ (создавали через аллокатор)
	for(int i = 0; i < (int)m_data_.size(); i++)
	{
		m_data_[i]->~zHtmlItem_old();
	}

	m_items_allocator.FreeAll();
	m_str_pool.FreeAll();

	m_data_.clear();

	free(m_main_str);
	m_main_str = 0;
}


//------------------------------------------------------------------------------------------
char *zHtml_old::create_main_str(int size)
{
	if(m_main_str)
		return (_ass(0), 0);

	m_main_str = (char*)malloc(sizeof(char) * (size + 1));

	return m_main_str;
}


//------------------------------------------------------------------------------------------
//добавить item
const zHtmlItem_old &zHtml_old::add_item(const std::string &str, HTML_ITEM_TYPE type)
{
	//static zHtmlItem_old _test;
	//return _test;

	zHtmlItem_old *buf = (zHtmlItem_old*)m_items_allocator.GetMemory();

#pragma push_macro("new")
#undef new
	::new(buf) zHtmlItem_old();
#pragma pop_macro("new")

	buf->set_parent(this);
	buf->create_from_string(str, type);
	m_data_.push_back(buf);

	return *buf;
}


//------------------------------------------------------------------------------------------
//добавить item
const zHtmlItem_old &zHtml_old::add_item(const zHtmlItem_old &item)
{
	zHtmlItem_old *buf = (zHtmlItem_old*)m_items_allocator.GetMemory();

#pragma push_macro("new")
#undef new
	::new(buf) zHtmlItem_old();
#pragma pop_macro("new")

	*buf = item;
	buf->set_parent(this);
	m_data_.push_back(buf);

	return *buf;
}


//------------------------------------------------------------------------------------------
//вставить item со сдвигом, на место ind
void zHtml_old::insert_item(const zHtmlItem_old &item, int ind)
{
	zHtmlItem_old *buf = (zHtmlItem_old*)m_items_allocator.GetMemory();

#pragma push_macro("new")
#undef new
	::new(buf) zHtmlItem_old();
#pragma pop_macro("new")

	*buf = item;
	buf->set_parent(this);
	m_data_.insert(m_data_.begin() + ind, buf);
}


//------------------------------------------------------------------------------------------
//выставляем всем тегам флаг вывода в файл
void zHtml_old::set_print_all()
{
	for(int i = 0; i < size(); i++)
	{
		zHtmlItem_old &itm = item(i);
		itm.set_dont_print(false);

		vector<zHtmlValue_old> & values = itm.get_values();
		for(int j = 0; j < (int)values.size(); j++)
			values[j].set_hidden(false);
	}
}


//------------------------------------------------------------------------------------------
//не выводить комментарии
//возвращает количество спрятанных
int zHtml_old::hide_comments()
{
	int dbg_count = 0;

	for(int i = 0; i < size(); i++)
	{
		zHtmlItem_old &itm = item(i);
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
int zHtml_old::hide_tag(std::string tag)
{
	textutl::to_lower_eng(tag, 0);

	int dbg_open = 0;
	int dbg_close = 0;

	for(int i = 0; i < size(); i++)
	{
		zHtmlItem_old &itm = item(i);
		if(itm.is_tag(tag))
		{
			itm.set_dont_print(true);
			if(itm.is_close_tag())
				dbg_close++;
			else
				dbg_open++;
		}
	}

	rel_assert(dbg_open == dbg_close || tag == "meta");

	return dbg_open + dbg_close;
}


//------------------------------------------------------------------------------------------
//не выводить в файл тэг tag вместе с закрывающим и текстом между ними
//возвращает количество спрятанных
int zHtml_old::hide_tag_with_text(std::string tag)
{
	textutl::to_lower_eng(tag, 0);

	int dbg_open = 0;
	int dbg_close = 0;

	int opened = 0;
	for(int i = 0; i < size(); i++)
	{
		zHtmlItem_old &itm = item(i);
		if(itm.is_tag(tag))
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
int zHtml_old::hide_param(std::string tag, std::string param)
{
	textutl::to_lower_eng(tag, 0);

	int dbg_cnt = 0;
	bool in_noindex = false;

	for(int i = 0; i < size(); i++)
	{
		zHtmlItem_old &itm = item(i);

		if(itm.is_tag("noindex"))
		{
			in_noindex = !itm.is_close_tag();
			continue;
		}

		if(m_skip_noindex && in_noindex)
			continue;

		if(tag.empty() || itm.is_tag(tag))
		{
			vector<zHtmlValue_old> & values = itm.get_values();
			for(int j = 0; j < (int)values.size(); j++)
			{
				if(values[j].name() == param)
				{
					values[j].set_hidden(true);
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
int zHtml_old::replace_tag_param(std::string tag, std::string param, std::string new_str)
{
	textutl::to_lower_eng(tag, 0);

	bool use_format = (new_str.find("%d") != string.npos);

	bool in_noindex = false;

	int cnt = 0;
	for(int i = 0; i < size(); i++)
	{
		zHtmlItem_old &itm = item(i);

		if(itm.is_tag("noindex"))
		{
			in_noindex = !itm.is_close_tag();
			continue;
		}

		if(m_skip_noindex && in_noindex)
			continue;

		if(itm.is_tag(tag))
		{
			vector<zHtmlValue_old> & values = itm.get_values();
			for(int j = 0; j < (int)values.size(); j++)
			{
				if(values[j].name() == param)
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
			}
		}
	}

	return cnt;
}


//------------------------------------------------------------------------------------------
//удаляет пустые строки и ненужные пробельные символы
//обрабатывает только HTML_TEXT
int zHtml_old::remove_blank_lines_and_trim()
{
	bool prev_was_empty = false;
	for(int i = 0; i < size(); i++)
	{
		zHtmlItem_old &itm = item(i);

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
			prev_was_empty = itm.get_dont_print();
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
int zHtml_old::hide_param_value_with_text(std::string tag, std::string param, std::string value, bool add_quotes, bool sub_str,
										bool hide_text)
{
	textutl::to_lower_eng(tag, 0);

	if(add_quotes)
		value = '"' + value + '"';

	int dbg_open = 0;
	int dbg_close = 0;

	int opened = 0;
	for(int i = 0; i < size(); i++)
	{
		zHtmlItem_old &itm = item(i);
		if(!itm.is_tag(tag))
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
				bool t = false;	//тэг нам подходит

				vector<zHtmlValue_old> & values = itm.get_values();
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
			}
		}
	}

	rel_assert(dbg_open == dbg_close || !hide_text);
	rel_assert(opened == 0);

	return dbg_open + dbg_close;
}


//------------------------------------------------------------------------------------------
//добавить к началу или в конец param у всех тэгов tag строку new_str
//если to_end, то добавляем в конец, иначе - в начало
//если not_add_if_has, то не добавляем add_str, если она уже содержится в value
//использует m_skip_noindex
int zHtml_old::add_to_tag_param(string tag, string param, string add_str, bool to_end, bool not_add_if_has)
{
	textutl::to_lower_eng(tag, 0);

	bool in_noindex = false;

	int cnt = 0;
	for(int i = 0; i < size(); i++)
	{
		zHtmlItem_old &itm = item(i);

		if(itm.is_tag("noindex"))
		{
			in_noindex = !itm.is_close_tag();
			continue;
		}

		if(m_skip_noindex && in_noindex)
			continue;

		if(itm.is_tag(tag))
		{
			vector<zHtmlValue_old> & values = itm.get_values();
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
		}
	}

	return cnt;
}


//------------------------------------------------------------------------------------------
int zHtml_old::hide_value_with_substr(std::string tag, std::string param, std::string value)
{
	int cnt = 0;
	for(int i = 0; i < size(); i++)
	{
		zHtmlItem_old &itm = item(i);
		if(tag.empty() || itm.is_tag(tag))
		{
			vector<zHtmlValue_old> & values = itm.get_values();
			for(int j = 0; j < (int)values.size(); j++)
			{
				if(values[j].name() == param)
				{
					if(values[j].val().find(value) != string.npos)
					{
						values[j].set_val("");
						cnt++;
					}
				}
			}
		}
	}

	return cnt;
}


//------------------------------------------------------------------------------------------
//заменяет все текста для href на рандомные текста из file_name
int zHtml_old::replace_ahref_titles(const wchar_t *file_name)
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
		zHtmlItem_old &itm = item(i);
		if(itm.is_tag("a"))
			in_a = !itm.is_close_tag();

		if(!in_a)
			continue;

		if(itm.get_type() == HTML_TEXT)
		{
			string s = itm.get_str_();
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

				itm.create_from_string(zstr::ansi_to_utf8(new_s), HTML_TEXT);
			}
		}
	}

	return cnt;
}

