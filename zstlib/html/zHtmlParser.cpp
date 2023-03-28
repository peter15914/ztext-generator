#include "stdafx.h"

#include <zstlib/zutl.h>
#include <zstlib/html/zCss.h>
#include <zstlib/utl/zRWArray.h>
#include <zstlib/zsubstr.h>
#include <zstlib/textutl.h>

#include "zHtmlParser.h"
#include "zHtml.h"
#include "zHtmlItem.h"



#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

//режим расширенной отладки
//#define EXTENDED_DEBUG_MODE

//------------------------------------------------------------------------------------------
zHtmlParser::zHtmlParser() :
	m_global_bads_file(0),
	m_global_text_file(0)
{
}


//------------------------------------------------------------------------------------------
zHtmlParser::~zHtmlParser()
{
	if(m_global_bads_file)
		fclose(m_global_bads_file);
	if(m_global_text_file)
		fclose(m_global_text_file);
}


//------------------------------------------------------------------------------------------
//парсит файл в объект html
bool zHtmlParser::parse_html_file(const wchar_t *file_name, zHtml& html)
{
	//размер файла
	int size = (int)zutl::get_file_size(file_name);
	if(!size)
		return (rel_assert(0), false);

	char *main_str = html.create_main_str(size);

	//читаем весь файл в память
	bool ok = zRWArray::read_file_to_str(file_name, main_str, size);
	if(!ok)
		return false;

	return _parse_html_string(main_str, html, size);
}


//------------------------------------------------------------------------------------------
bool zHtmlParser::parse_html_string_(char *buf, int size, zHtml &html, bool create_copy)
{
	if(create_copy)
	{
		char *main_str = html.create_main_str(size);
		strcpy(main_str, buf);
		return _parse_html_string(main_str, html, size);
	}
	else
		return _parse_html_string(buf, html, size);
}


//------------------------------------------------------------------------------------------
bool zHtmlParser::parse_html_string_slow(std::string buf, zHtml &html)
{
	int size = buf.size();

	char *main_str = html.create_main_str(size);
	strcpy(main_str, buf.c_str());

	return _parse_html_string(main_str, html, size);
}


//------------------------------------------------------------------------------------------
//#define DEBUG_TEST_NO_ADD

//------------------------------------------------------------------------------------------
//парсит html-строку в объект html
bool zHtmlParser::_parse_html_string(char *data, zHtml &html, int size)
{
	_ass(html.size() == 0);

	//в режиме плохого тэга, надо считать текстом всё до тех пор, пока он не закроется
	//теперь там нет открывающего '/'
	zsubstr bad_tag;
	bad_tag.init();

	//пробегаем весь файл и парсим
	for(int ii = 0; ii < size; )
	{
	#ifdef _DEBUG
		//int dbg_prev_ii = ii;
		//const char *dbg_str = &data[ii];
		//UNREFERENCED_PARAMETER(dbg_prev_ii);
		//UNREFERENCED_PARAMETER(dbg_str);
		//if(ii == 3639)
		//{
		//	int u = 0;
		//}
	#endif

		//ii++;continue;

		if(bad_tag.sz && data[ii] == '<' && data[ii+1] == '/')
		{
			//в режиме плохого тэга пытаемся найти закрывающий
			if(ii + 3 + bad_tag.sz < size && bad_tag.equal_nc_sz(&data[ii+2]))
			{
				int jj = zstr::get_first_non_letter_or_num_tag(data, ii+2 + bad_tag.sz);
				if(jj == -1 || data[jj] != '>')
				{
					_ass(0);	//так не должно быть, корявая html
					//но ладно, всё будет добавляться в текст
				}
				else
				{
					//всё нормально, закрывающий тэг попался
					bad_tag.set(data + ii + 1, bad_tag.sz + 1);

					#ifndef DEBUG_TEST_NO_ADD
						html.add_item(bad_tag, HTML_TAG);
					#endif
					bad_tag.clear();
					ii = jj + 1;
				}
			}
		}

		if(!bad_tag.sz && data[ii] == '<')
		{
			//коммент или тэг
			if(ii+3 < size && data[ii+1] == '!' && data[ii+2] == '-' && data[ii+3] == '-')
			{
				//коммент
				//в комментариях кавычки пропускаем
				//скорей всего тут будет ошибка на яваскрипте - потом надо сделать чтоб работало хорошо во всех случаях
				//(как только попадется еще раз пример с яваскриптом)
				zsubstr s2_comment;

				int kk = zstr::find_str(data, "-->", ii+1);
				if(kk != -1)
				{
					s2_comment.set(data, ii + 4, kk);
					ii = kk + 3;
				}
				else
				{
					//нет закрывающего тэга, весь оставшийся файл - закомменчен
					_ass(0);	//ассертнемся на всякий случай
					s2_comment.set(data, ii + 4, size);
					ii = size;
				}

				#ifndef DEBUG_TEST_NO_ADD
					html.add_item(s2_comment, HTML_COMMENT);
				#endif
			}
			else
			{
				//тэг
				zsubstr s2_tag;

				int kk = find_ignore_const_strs_(data, ">", ii+1);
				if(kk != -1)
				{
					s2_tag.set(data, ii + 1, kk);
					ii = kk + 1;
				}
				else
				{
					_ass(0);	//нет закрывающего '>', ассертнемся на всякий случай
					s2_tag.set(data, ii + 1, size);
					ii = size;
				}

				//добавляем
				#ifndef DEBUG_TEST_NO_ADD
					const zHtmlItem &itm = html.add_item(s2_tag, HTML_TAG);

					//если добавили плохой тэг, надо посчитать текстом всё до тех пор, пока он не закроется
					if(itm.is_bad_tag() && !itm.is_close_tag())
						bad_tag.set(s2_tag.str, itm.m_tag.sz);
				#endif
			}
		}//if(!bad_tag_mode && buf[ii] == '<')
		else
		{
			//это не ошибка, действительно в режиме плохого тега всё в виде текста пихается

			//тут текст идет
			//в тексте свободно могут встречаться кавычки, на них не обращаем внимания
			int jj = zstr::find_str(data, "<", ii+1);
			zsubstr s2_text;
			if(jj != -1)
			{
				s2_text.set(data, ii, jj);
				ii = jj;
			}
			else
			{
				//больше нет тэгов
				s2_text.set(data, ii, size);
				ii = size;
			}

			#ifndef DEBUG_TEST_NO_ADD
				html.add_item(s2_text, HTML_TEXT);
			#endif
		}
	}


	return true;
}


//------------------------------------------------------------------------------------------
//сохранить в файл
//если auto_format, то переформатируем при выводе
bool zHtmlParser::save_to_file(const wchar_t *file_name, zHtml& html, bool auto_format, const wchar_t *append_file_name)
{
	UNREFERENCED_PARAMETER(auto_format);

	FILE *f = _wfopen(file_name, L"wt");
	if(!f)
		return (rel_assert(0), false);

	bool prev_has_new_line = true;

	//выводим в файл
	for(int i = 0; i < html.size(); i++)
	{
		zHtmlItem &itm = html.item(i);
		if(itm.is_dont_print())
			continue;

		string str = itm.get_print_string_slow_();
		if(!str.empty())
		{
			if(itm.get_type() == HTML_TAG && !itm.is_close_tag())
			{
				if(!prev_has_new_line)
					fprintf(f, "\n");
			}
			fprintf(f, "%s", str.c_str());
		}

		prev_has_new_line = (str.find('\n') != string.npos);
	}

	//
	if(append_file_name && wcslen(append_file_name) > 0)
	{
		FILE *file2 = _wfopen(append_file_name, L"rt");
		if(!file2)
			(_ass(0));
		else
		{
			//теперь добавляем append_file_name
			static char buf[10000];
			while(fgets(buf, sizeof(buf), file2))
				fprintf(f, "%s", buf);
			fclose(file2);
		}
	}

	fclose(f);
	return true;
}


//------------------------------------------------------------------------------------------
//сохранить в строку
void zHtmlParser::save_to_str(zHtml &html, std::string &ret_str)
{
	ret_str.clear();
	for(int i = 0; i < html.size(); i++)
	{
		zHtmlItem &itm = html.item(i);
		if(itm.is_dont_print())
			continue;

		ret_str += itm.get_print_string_slow_();
	}
}


//------------------------------------------------------------------------------------------
bool zHtmlParser::save_texts_to_change(const wchar_t *file_name, zHtml& html)
{
	set<string> g_strings_not_change;
	g_strings_not_change.insert("&nbsp;");

	FILE *f = _wfopen(file_name, L"wt");
	if(!f)
		return (rel_assert(0), false);

	//глобальный текстовый файл
	if(!m_global_text_file_name.empty() && !m_global_text_file)
	{
		zutl::MakeBackups(m_global_text_file_name.c_str(), 3);
		m_global_text_file = _wfopen(m_global_text_file_name.c_str(), L"wt");
		_ass(m_global_text_file);
	}

	if(m_global_text_file)
		fprintf(m_global_text_file, ";------------%S\n", file_name);

	//
	set<string> norm_texts;
	zutl::text_file_to_set(L"data/a_href_titles.txt", norm_texts);

	//выводим в файл
	for(int i = 0; i < html.size(); i++)
	{
		zHtmlItem &itm = html.item(i);

		if(!itm.is_dont_print() && itm.get_type() == HTML_TEXT)
		{
			string str = itm.get_print_string_slow_();
			zstr::trim(str);

			if(!str.empty())
			{
				if(g_strings_not_change.find(str.c_str()) == g_strings_not_change.end())
				{
					fprintf(f, "%s\n", str.c_str());
					if(m_global_text_file && norm_texts.find(zstr::utf8_to_ansi(str)) == norm_texts.end())
						fprintf(m_global_text_file, "%s\n", str.c_str());
				}
			}
		}
	}

	fclose(f);
	return true;
}


//------------------------------------------------------------------------------------------
//сохраняет в файл список оставшихся href'ов
bool zHtmlParser::save_https_left(const wchar_t *file_name, zHtml& html)
{
	FILE *f = _wfopen(file_name, L"wt");
	if(!f)
		return (rel_assert(0), false);

	//
	if(!m_global_bads_file_name.empty() && !m_global_bads_file)
	{
		zutl::MakeBackups(m_global_bads_file_name.c_str(), 3);
		m_global_bads_file = _wfopen(m_global_bads_file_name.c_str(), L"wt");
		_ass(m_global_bads_file);
	}

	if(m_global_bads_file)
		fprintf(m_global_bads_file, ";------------%S\n", file_name);

	//
	const char *bad_strs[] = {"iframe", "onerror", "onfocus", "this.", "data-src", "onkey", "javascript", "base64",
								"addthis", "script", "onmouse", "onclick", "onload", "data-href", "java"};

	//выводим в файл
	for(int i = 0; i < html.size(); i++)
	{
		zHtmlItem &itm = html.item(i);
		if(itm.is_dont_print())
			continue;

		string str = itm.get_print_string_slow_();
		zstr::trim(str);

		bool bad = false;
		string bad_str;

		if(itm.get_type() == HTML_TAG || itm.get_type() == HTML_TEXT)
		{
			if(str.find("http:") != string.npos)
			{
				if(!(itm.get_flags_() & HF_STRANGE_TAG))
				{
					bad = true;
					bad_str = (boost::format("(http:) : %s\n") % str.c_str()).str();
				}
			}
		}

		if(!bad && itm.get_type() == HTML_TAG)
		{
			for(int ii = 0; ii < _countof(bad_strs); ii++)
			{
				//цикл, потому что некоторые bad на самом деле не bad
				for(int kk = 0; kk < (int)str.length(); kk++)
				{
					size_t jj = str.find(bad_strs[ii], kk);
					if(jj == string.npos)
						break;

					bad = true;

					//вдруг это не script, а description или что-то похожее
					if(strcmp(bad_strs[ii], "script") == 0)
					{
						if(jj > 1 && str[jj-2] == 'd' && str[jj-1] == 'e')
							bad = false;
					}

					if(bad)
						break;

					kk = (int)jj;
				}

				if(bad)
				{
					bad_str = (boost::format("(%s) : %s\n") % bad_strs[ii] % str.c_str()).str();
					break;
				}
			}
		}

		//если плохой, выводим в лог
		if(bad)
		{
			fputs(bad_str.c_str(), f);
			if(m_global_bads_file)
				fputs(bad_str.c_str(), m_global_bads_file);
		}
	}

	fclose(f);
	return true;
}


//------------------------------------------------------------------------------------------
//удаляет комментарии
void zHtmlParser::_parse_css_cut_comments(string &buf)
{
	string ret;
	int size = (int)buf.size();
	ret.reserve(size);

	bool is_strconst = false;	//режим строковой константы
	char strconst_char = 0;
	bool is_comment = false;	//режим комментария
	int last_used_next = 0;	//символ, следующий за последним использованным

	//пробегаем весь файл и парсим
	for(int ii = 0; ii < size; )
	{
		//в режиме комментария
		if(is_comment)
		{
			if(buf[ii] == '*' && ii+1 < size && buf[ii+1] == '/')
			{
				is_comment = false;
				ii += 2;
				last_used_next = ii;
			}
			else
				ii++;

			continue;
		}

		//в режиме строковой константы
		if(is_strconst)
		{
			if(strconst_char == buf[ii])
				is_strconst = false;
			ii++;
			continue;
		}

		//вдруг начинается коммент
		if(buf[ii] == '/' && ii+1 < size && buf[ii+1] == '*')
		{
			ret += buf.substr(last_used_next, ii - last_used_next);

			is_comment = true;
			last_used_next = ii;
			ii += 2;

			continue;
		}

		//вдруг начинается строковая константа
		if(buf[ii] == '\'' || buf[ii] == '"')
		{
			is_strconst = true;
			strconst_char = buf[ii];
			ii++;

			continue;
		}

		ii++;
	}

	ret += buf.substr(last_used_next, size - last_used_next);

	buf = ret;
}


//------------------------------------------------------------------------------------------
//парсит файл в объект css
bool zHtmlParser::parse_css_file(const wchar_t *file_name, zCss& css)
{
	string buf;

	//читаем весь файл в память
	bool ok = zRWArray::read_file_to_str(file_name, buf);
	if(!ok)
		return false;

	_parse_css_cut_comments(buf);
	//_write_file_from_mem((wstring(file_name) + L"_bak").c_str(), buf);


	//bool is_strconst = false;	//режим строковой константы
	//char strconst_char = 0;
	//bool in_selector = false;	//находимся внутри селектора

	//пробегаем весь файл и парсим
	int size = (int)buf.size();
	for(int ii = 0; ii < size; )
	{
		int kk = find_ignore_const_strs_slow(buf, "{", ii);
		if(kk == -1)
		{
			//больше нет селекторов, уходим
			string s = buf.substr(ii, size);
			zstr::trim(s);
			if(!s.empty())
			{
				_warn("ничего не должно остаться: " + s);
			}
			break;
		}

		//
		int kk2 = find_ignore_const_strs_slow(buf, "}", kk+1);
		if(kk2 == -1)
		{
			_ass(0);
			break;
		}

		//
		string s = buf.substr(ii, kk - ii);
		zstr::trim(s);
		css.add_item(CSS_SELECTOR, s);

		//собираем свойства
		int jj = kk + 1;
		while(jj < kk2)
		{
			int tt1 = find_ignore_const_strs_slow(buf, ":", jj);
			if(tt1 == -1 || tt1 > kk2)
				break;
			int tt2 = find_ignore_const_strs_slow(buf, ";", tt1 + 1, "\'\'\"\"()");
			if(tt2 == -1 || tt2 > kk2)	//вдруг после последнего отсутствует
				tt2 = kk2;

			string name = buf.substr(jj, tt1 - jj);
			string val = buf.substr(tt1+1, tt2 - tt1 - 1);
			zstr::trim(name);
			zstr::trim(val);

			css.add_item(CSS_PARAM, name, val);
			jj = tt2 + 1;
		}
		if(jj < kk2)
		{
			string left = buf.substr(jj, kk2 - jj);
			zstr::trim(left);
			if(!left.empty())
			{
				_warn("ошметки в css: " + left);
			}
		}

		css.add_item(CSS_SELECTOR_END);

		ii = kk2 + 1;
	}


	return true;
}

//------------------------------------------------------------------------------------------
//сохранить в файл
bool zHtmlParser::save_css_to_file(const wchar_t *file_name, zCss& css)
{
	FILE *f = _wfopen(file_name, L"wt");
	if(!f)
		return (rel_assert(0), false);

	vector<zCssItem> &data = css.get_data();

	//выводим в файл
	for(int i = 0; i < (int)data.size(); i++)
	{
		string str = data[i].get_print_string();
		if(!str.empty())
			fprintf(f, "%s\n", str.c_str());
	}

	fclose(f);
	return true;
}


//открыть глобальный файл, в который пишутся все те же данные, что и в локальные hrefs_left_index.txt
void zHtmlParser::set_global_bads_file(const wchar_t *file_name, bool text_files)
{
	if(!text_files)
		m_global_bads_file_name = file_name;
	else
		m_global_text_file_name = file_name;
}


//почистить заголовочную часть
//действуем по принципу "лучше удалить что-то лишнее, чем оставтиь что-то плохое"
void zHtmlParser::clean_head(zHtml& html)
{
	bool in_head = false;
	bool in_title = false;
	bool in_style = false;
	bool in_script = false;
	int head_close_ind = INT_MAX;
	bool was_php = false;

	//выводим в файл
	for(int i = 0; i < html.size(); i++)
	{
		zHtmlItem &itm = html.item(i);
		if(itm.is_dont_print())
			continue;

		if(itm.get_type() != HTML_TAG)
		{
			if(in_head)
			{
				if(in_title)
				{
					_ass(itm.get_type() == HTML_TEXT);
					//itm.create_from_string("zzzTitle", HTML_TEXT);
					//прячем
					itm.set_dont_print(true);
				}
				else
				if(in_style)
				{
					//прячем
					itm.set_dont_print(true);
				}
				else
				if(in_script)
				{
					//прячем
					itm.set_dont_print(true);
				}
				else
				{
					_ass(0);	//TODO!!!
					string ss;// = itm.get_str_();
					zstr::trim(ss);
					if(!ss.empty())
					{
						if(ss.substr(0, 4) == "?php")
							was_php = true;
						else
							(_ass(0));
					}
				}
			}
			continue;
		}

		if(itm.is_tag("head"))
		{
			if(itm.is_close_tag())
			{
				head_close_ind = i;
				break;
			}

			in_head = true;
			continue;
		}

		if(!in_head)
			continue;

		
		if(itm.is_tag("title"))
		{
			if(itm.is_close_tag())
				in_title = false;
			else
				in_title = true;
			continue;
		}

		if(itm.is_tag("style"))
		{
			if(itm.is_close_tag())
				in_style = false;
			else
				in_style = true;
			continue;
		}

		if(itm.is_tag("script"))
		{
			itm.set_dont_print(true);
			if(itm.is_close_tag())
				in_script = false;
			else
				in_script = true;
			continue;
		}


		bool good = false;
		//действуем по принципу "лучше удалить что-то лишнее, чем оставить что-то плохое"
		if(itm.is_tag("meta") || itm.is_tag("div"))
		{
		}
		else
		if(itm.is_tag("link"))
		{
			if(itm.has_value_("rel"))
			{
				zsubstr val = itm.get_value_("rel");
				if(val.equal_nc("stylesheet"))
					good = true;
			}
		}
		else
			(_ass(0));

		if(!good)
			itm.set_dont_print(true);

	}

	if(head_close_ind == INT_MAX)
		return (_ass(0));

	//добавляем
	if(!was_php)
	{
		zHtml h_add;
		this->parse_html_string_slow("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
				"<meta http-equiv=\"Content-Language\" content=\"ru\">", h_add);

		int sz = (int)h_add.size();
		for(int i = 0; i < sz; i++)
		{
			zHtmlItem &item = h_add.item(sz - i - 1);
			html.insert_item_slow(item, head_close_ind);
		}
	}
}


//отладочное сохранение в файл (для юнит-тестов)
bool zHtmlParser::save_to_file_dbg(const wchar_t *file_name, zHtml& html)
{
	FILE *f = _wfopen(file_name, L"wt");
	if(!f)
		return (_ass(0), false);

	//выводим в файл
	for(int i = 0; i < html.size(); i++)
	{
		zHtmlItem &itm = html.item(i);
		string str = itm.get_print_string_slow_();

		if(itm.is_tag())
			textutl::to_lower_eng(str, 0);

		HTML_ITEM_TYPE t = itm.get_type();
		string type = (t == HTML_COMMENT) ? "cmnt" : (t == HTML_TAG) ? "tag" : (t == HTML_TEXT) ? "txt" : (t == HTML_PHP) ? "php" : (_ass(0), "");

		fprintf(f, "\n#%d#%s#%s#%d#", i, type.c_str(), str.c_str(), i);
	}

	fclose(f);
	return true;
}

