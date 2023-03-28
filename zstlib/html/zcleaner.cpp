#include "stdafx.h"

#include "zcleaner.h"

#include <zstlib/utl/zLog.h>
#include <zstlib/zutl.h>
//#include <zstlib/utl/zRWArray.h>
#include <zstlib/html/zCss.h>

#include <zstlib/html/zHtmlItem.h>

#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace std;


zcleaner::zcleaner() :
	m_backups_depth(0)
{
}


zcleaner::~zcleaner()
{
}


//выполнить чистку
//версия, кот. работает через cpp (старая)
void zcleaner::do_clean()
{
}


bool zcleaner::parse_html_file(const char *_file_name)
{
	wstring file_name = zstr::s_to_w(_file_name);
	return m_parser.parse_html_file(file_name.c_str(), *this);
}


bool zcleaner::save_texts_to_change(const char *_file_name)
{
	wstring file_name = zstr::s_to_w(_file_name);
	return m_parser.save_texts_to_change(file_name.c_str(), *this);
}


bool zcleaner::save_https_left(const char *_file_name)
{
	wstring file_name = zstr::s_to_w(_file_name);
	return m_parser.save_https_left(file_name.c_str(), *this);
}


bool zcleaner::save_to_file(const char *_file_name, const char *_append_file_name)
{
	wstring file_name = zstr::s_to_w(_file_name);
	wstring append_file_name = zstr::s_to_w(_append_file_name);

	if(m_backups_depth > 0)
		zutl::MakeBackupsEx(file_name, m_backups_depth, L"_bak", 2);

	return m_parser.save_to_file(file_name.c_str(), *this, true, append_file_name.c_str());
}


void zcleaner::clean_css_file(const wchar_t *_file_name, int add_rand_items, int hide_attrs_with_http)
{
	zdebug::LogImp("zcleaner::clean_css_file: " + zstr::w_to_s(_file_name));

	wstring file_name = _file_name;

	zCss css;

	zHtmlParser html_parser;
	html_parser.parse_css_file(file_name.c_str(), css);

	if(hide_attrs_with_http)
		css.hide_attrs_with_http();
	if(add_rand_items)
		css.add_rand_items();

	wstring res_file_name = file_name;	//file_name + L"_res";
	if(m_backups_depth > 0)
		zutl::MakeBackupsEx(res_file_name, m_backups_depth, L"_bak", 2);
	html_parser.save_css_to_file(res_file_name.c_str(), css);
}


void zcleaner::clean_css_dir(const wchar_t *_work_dir, int add_rand_items, int hide_attrs_with_http)
{
	if(!_work_dir || !_work_dir[0])
		return (_ass(0));

	vector<wstring> files;
	zutl::FindFilesInFolder(_work_dir, L"*.css", files);

	wstring work_dir = _work_dir;

	wchar_t last = work_dir[work_dir.length() - 1];
	if(last != L'/' && last != L'\\')
		work_dir += L'/';

	for(int i = 0; i < (int)files.size(); i++)
		clean_css_file((work_dir + files[i]).c_str(), add_rand_items, hide_attrs_with_http);
}


//копирует вспомогательные файлы в папку с почищенным html
void zcleaner::copy_some_files(const char *work_dir, const char *file_name, bool replace)
{
	wstring dir = zstr::s_to_w(work_dir);
	wstring file = zstr::s_to_w(file_name);

	wstring file_src = L"data/" + file;
	wstring file_dest = dir + L"/" + file;

	if(replace)
		zutl::MakeBackupsEx(file_dest.c_str(), 5, L"_bak", 0);

	//копируем, заменяя или нет в зависимости от replace
	CopyFileW(file_src.c_str(), file_dest.c_str(), !replace);
}


void zcleaner::clean_head()
{
	m_parser.clean_head(*this);
}


//вставить рандомные пустые теги
void zcleaner::insert_rand_tags()
{
	_ass(0);	//неизвестно, работает ли после переделок

	zHtml buf_html;

	int sz = this->size();
	buf_html.reserve(sz);

	int to_place = sz / 20;
	if(to_place < 5)
		to_place = 5;
	if(to_place > 30)
		to_place = 30;

	//примерно каждые per_tag тэгов нужно вставлять
	int per_tag = sz / to_place;
	if(per_tag < 5)
		per_tag = 5;

	bool in_body = false;
	bool need_to_place = false;	//если true, то надо вставлять после след. </div>
	int placed_cnt = 0;
	int prev_place = 0;

	for(int i = 0; i < sz; i++)
	{
		zHtmlItem &itm = item(i);
		buf_html.pushback_item_slow(itm);

		if(!itm.is_tag())
			continue;

		if(itm.is_tag("body"))
		{
			in_body = !itm.is_close_tag();
			continue;
		}

		if(!in_body)
			continue;

		if(need_to_place && placed_cnt < to_place)
		{
			//надо вставлять после </div>
			if(itm.is_close_tag("div"))
			{
				zHtmlItem itm1, itm2;
				_ass(0);	//todo
				//itm1.create_from_string("div id=\"" + textutl::get_rand_str(8)+ "\"", HTML_TAG);
				//itm2.create_from_string("/div", HTML_TAG);
				buf_html.pushback_item_slow(itm1);
				buf_html.pushback_item_slow(itm2);

				need_to_place = false;
				placed_cnt++;
				prev_place = i;
			}
		}
		else
		if(i - prev_place > per_tag)
		{
			need_to_place = true;
		}
	}

	_ass(0);	//todo
	//m_data_.swap(buf_html.get_data_for_swap());
}

