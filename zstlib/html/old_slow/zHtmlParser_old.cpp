#include "stdafx.h"

#include <zstlib/zutl.h>
#include <zstlib/html/zCss.h>
#include <zstlib/utl/zRWArray.h>
#include <zstlib/textutl.h>

#include "zHtmlParser_old.h"
#include "zHtml_old.h"



#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;
using namespace h_old;


//режим расширенной отладки
//#define EXTENDED_DEBUG_MODE

//------------------------------------------------------------------------------------------
zHtmlParser_old::zHtmlParser_old() :
	m_global_bads_file(0),
	m_global_text_file(0)
{
}


//------------------------------------------------------------------------------------------
zHtmlParser_old::~zHtmlParser_old()
{
	if(m_global_bads_file)
		fclose(m_global_bads_file);
	if(m_global_text_file)
		fclose(m_global_text_file);
}


//------------------------------------------------------------------------------------------
//находит первое вхождение sub_str в str, начиная с nBeg, исключая подстроки, находящиеся в
//константных строках (строки в двойных или одинарных кавычках)
//ТЕПЕРЬ ПЕРЕНОС СТРОКИ НЕ ВЫКЛЮЧАЕТ режим константной строки. //old version: перенос строки выключает режим константной строки, т.к. это косяк в html, строковые константы не могут быть на несколько строк файла
//limits - строка, содержащая пары символов - ограничителей (обычно(по дефолту) два вида кавычек, но иногда и скобки)
int zHtmlParser_old::find_ignore_const_strs(const string &str, const string &sub_str, int nBeg, string limits)
{
	if(sub_str.empty() || limits.empty())
		return (rel_assert(0), -1);

	int sz = (int)str.size();
	int sz2 = (int)sub_str.size();

	//первый символ в искомой строке
	char csrch = sub_str[0];

	char curs_str = 0;	//одинарная или двойная кавычка, означает какой режим сейчас
	for(int ii = nBeg; ii < sz - (sz2-1); ii++)
	{
		char c = str[ii];

		if(curs_str)
		{
			//if(c == '\n')		//перенос строки выключает режим константной строки
			//	curs_str = 0;

			if(c == curs_str)	//закрывающая кавычка
				curs_str = 0;

			//эта часть - внутри константной строки, пропускаем
			continue;
		}

		for(int kk = 0; kk < (int)limits.size(); kk++)
		{
			char lim_c = limits[kk];
			if(c == lim_c)
			{
				curs_str = limits[kk+1];
				break;
			}
		}

		//началась константная строка, пропускаем
		if(curs_str)
			continue;


		if(c == csrch)
		{
			int jj = 1;
			for(; jj < sz2; jj++)
			{
				if(sub_str[jj] != str[ii + jj])
					break;
			}

			if(jj == sz2)	//все символы равны
				return ii;
		}
	}
	return -1;
}


//------------------------------------------------------------------------------------------
//парсит файл в объект html
bool zHtmlParser_old::parse_html_file_old_slow(const wchar_t *file_name, zHtml_old& html)
{
	string buf;

	//читаем весь файл в память
	bool ok = zRWArray::read_file_to_str(file_name, buf);
	if(!ok)
		return false;

	return parse_html_string_old_slow(buf, html);
}


//------------------------------------------------------------------------------------------
//парсит html-строку в объект html
//взята с ревизии 1111
bool zHtmlParser_old::parse_html_string_old_slow(const std::string &buf, zHtml_old &html)
{
	html.clear();

	//в режиме плохого тэга, надо считать текстом всё до тех пор, пока он не закроется
	bool bad_tag_mode = false;
	string close_bad_tag;

	//пробегаем весь файл и парсим
	int size = (int)buf.size();
	for(int ii = 0; ii < size; )
	{
	#ifdef _DEBUG
		int dbg_prev_ii = ii;
		const char *dbg_str = &buf[ii];
		UNREFERENCED_PARAMETER(dbg_prev_ii);	//suppress warning C4189
		UNREFERENCED_PARAMETER(dbg_str);		//suppress warning C4189
		//if(ii == 3639)
		//{
		//	int u = 0;
		//}
	#endif

		//ii++;continue;

		if(bad_tag_mode && buf[ii] == '<')
		{
			//в режиме плохого тэга пытаемся найти закрывающий
			int sz = (int)close_bad_tag.size();
			if(ii + sz < size && _strnicmp(&buf[ii+1], close_bad_tag.c_str(), sz) == 0)
			{
				int jj = zstr::get_first_non_letter_or_num_tag(buf.c_str(), ii+1 + sz);
				if(jj == -1 || buf[jj] != '>')
				{
					rel_assert(0);	//так не должно быть, корявая html
					//но ладно, всё будет добавляться в текст
				}
				else
				{
					//всё нормально, закрывающий тэг попался
					bad_tag_mode = false;
					html.add_item(close_bad_tag, HTML_TAG);
					ii = jj + 1;
				}
			}
		}

		if(!bad_tag_mode && buf[ii] == '<')
		{
			//коммент или тэг
			if(ii+3 < size && buf[ii+1] == '!' && buf[ii+2] == '-' && buf[ii+3] == '-')
			{
				//коммент
				//int kk = find_ignore_const_strs(buf, "-->", ii+1);
				//в комментариях кавычки пропускаем
				//скорей всего тут будет ошибка на яваскрипте - потом надо сделать чтоб работало хорошо во всех случаях
				//(как только попадется еще раз пример с яваскриптом)
				int kk = (int)buf.find("-->", ii+1);
				string s2;
				if(kk == -1)
				{
					//нет закрывающего тэга, весь оставшийся файл - закомменчен
					rel_assert(0);	//ассертнемся на всякий случай
					s2 = buf.substr(ii+4, buf.length());
					ii = (int)buf.length();
				}
				else
				{
					s2 = buf.substr(ii+4, kk-1 - (ii+4) + 1);
					ii = kk + 3;
				}

				html.add_item(s2, HTML_COMMENT);
			}
			else
			{
				//тэг
				int kk = find_ignore_const_strs(buf, ">", ii+1);
				string s2;
				if(kk == -1)
				{
					rel_assert(0);	//нет закрывающего '>', ассертнемся на всякий случай
					s2 = buf.substr(ii+1, buf.length());
					ii = (int)buf.length();
				}
				else
				{
					s2 = buf.substr(ii+1, kk-1 - (ii+1) + 1);
					ii = kk + 1;
				}

				//добавляем
				const zHtmlItem_old &itm = html.add_item(s2, HTML_TAG);

				if(itm.is_bad_tag() && !itm.is_close_tag())
				{
					//добавили плохой тэг, надо посчитать текстом всё до тех пор, пока он не закроется
					bad_tag_mode = true;
					close_bad_tag = itm.get_close_tag();
				}
			}
		}//if(!bad_tag_mode && buf[ii] == '<')
		else
		{
			//тут текст идет
			//в тексте свободно могут встречаться кавычки, на них не обращаем внимания
			int jj = (int)buf.find("<", ii+1);
			string s2;
			if(jj == -1)
			{
				//больше нет тэгов
				s2 = buf.substr(ii, buf.length());
				ii = (int)buf.length();
			}
			else
			{
				s2 = buf.substr(ii, jj - ii);
				ii = jj;
			}
			html.add_item(s2, HTML_TEXT);
		}
	}


	return true;
}


//------------------------------------------------------------------------------------------
//сохранить в файл
//если auto_format, то переформатируем при выводе
bool zHtmlParser_old::save_to_file(const wchar_t *file_name, zHtml_old& html, bool auto_format, const wchar_t *append_file_name)
{
	UNREFERENCED_PARAMETER(auto_format);

	FILE *f = _wfopen(file_name, L"wt");
	if(!f)
		return (rel_assert(0), false);

	bool prev_has_new_line = true;

	//выводим в файл
	for(int i = 0; i < html.size(); i++)
	{
		zHtmlItem_old &itm = html.item(i);
		string str = itm.get_print_string();

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
void zHtmlParser_old::save_to_str(zHtml_old &html, std::string &ret_str)
{
	ret_str.clear();
	for(int i = 0; i < html.size(); i++)
	{
		zHtmlItem_old &itm = html.item(i);
		ret_str += itm.get_print_string();
	}
}


//------------------------------------------------------------------------------------------
bool zHtmlParser_old::save_texts_to_change(const wchar_t *file_name, zHtml_old& html)
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
		zHtmlItem_old &itm = html.item(i);
		if(itm.get_type() == HTML_TEXT)
		{
			string str = itm.get_print_string();
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
bool zHtmlParser_old::save_https_left(const wchar_t *file_name, zHtml_old& html)
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
		zHtmlItem_old &itm = html.item(i);
		if(itm.get_dont_print())
			continue;

		string str = itm.get_print_string();
		zstr::trim(str);

		bool bad = false;
		string bad_str;

		if(itm.get_type() == HTML_TAG || itm.get_type() == HTML_TEXT)
		{
			if(str.find("http:") != string.npos)
			{
				if(!(itm.get_flags() & HF_DOCTYPE))
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
void zHtmlParser_old::_parse_css_cut_comments(string &buf)
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
bool zHtmlParser_old::parse_css_file(const wchar_t *file_name, zCss& css)
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
		int kk = find_ignore_const_strs(buf, "{", ii);
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
		int kk2 = find_ignore_const_strs(buf, "}", kk+1);
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
			int tt1 = find_ignore_const_strs(buf, ":", jj);
			if(tt1 == -1 || tt1 > kk2)
				break;
			int tt2 = find_ignore_const_strs(buf, ";", tt1 + 1, "\'\'\"\"()");
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
bool zHtmlParser_old::save_css_to_file(const wchar_t *file_name, zCss& css)
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
void zHtmlParser_old::set_global_bads_file(const wchar_t *file_name, bool text_files)
{
	if(!text_files)
		m_global_bads_file_name = file_name;
	else
		m_global_text_file_name = file_name;
}


//почистить заголовочную часть
//действуем по принципу "лучше удалить что-то лишнее, чем оставтиь что-то плохое"
void zHtmlParser_old::clean_head(zHtml_old& html)
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
		zHtmlItem_old &itm = html.item(i);
		if(itm.get_dont_print())
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
					string ss = itm.get_str_();
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
			if(itm.has_value("rel"))
			{
				string val = itm.get_value("rel");
				if(val == "stylesheet")
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
		zHtml_old h_add;
		this->parse_html_string_old_slow("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
				"<meta http-equiv=\"Content-Language\" content=\"ru\">", h_add);

		int sz = (int)h_add.size();
		for(int i = 0; i < sz; i++)
		{
			zHtmlItem_old &item = h_add.item(sz - i - 1);
			html.insert_item(item, head_close_ind);
		}
	}
}


//отладочное сохранение в файл (для юнит-тестов)
bool zHtmlParser_old::save_to_file_dbg(const wchar_t *file_name, zHtml_old& html)
{
	FILE *f = _wfopen(file_name, L"wt");
	if(!f)
		return (_ass(0), false);

	//выводим в файл
	for(int i = 0; i < html.size(); i++)
	{
		zHtmlItem_old &itm = html.item(i);
		string str = itm.get_print_string();

		if(itm.is_tag())
			textutl::to_lower_eng(str, 0);

		HTML_ITEM_TYPE t = itm.get_type();
		string type = (t == HTML_COMMENT) ? "cmnt" : (t == HTML_TAG) ? "tag" : (t == HTML_TEXT) ? "txt" : (t == HTML_PHP) ? "php" : (_ass(0), "");

		fprintf(f, "\n#%d#%s#%s#%d#", i, type.c_str(), str.c_str(), i);
	}

	fclose(f);
	return true;
}

