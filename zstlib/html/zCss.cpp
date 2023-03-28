#include "stdafx.h"

#include <zstlib/textutl.h>

#include "zCss.h"


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace std;


//------------------------------------------------------------------------------------------
zCss::zCss()
{
}


//------------------------------------------------------------------------------------------
zCss::~zCss()
{
}


//------------------------------------------------------------------------------------------
void zCss::add_item(CSS_ITEM_TYPE type, const string &s1/* = string()*/, const string &s2/* = string()*/)
{
	zCssItem buf;
	buf.s1 = s1;
	buf.s2 = s2;
	buf.type = type;

	m_data.push_back(buf);
}


//------------------------------------------------------------------------------------------
std::string zCssItem::get_print_string()
{
	if(hidden)
		return string();

	string ret;

	if(type == CSS_SELECTOR)
		ret = (boost::format("%s\n{") % s1).str();
	else
	if(type == CSS_PARAM)
	{
		ret = (boost::format("\t%s:%s;") % s1 % s2).str();
	}
	else
	if(type == CSS_SELECTOR_END)
	{
		ret = "}";
	}
	else
		(_ass(0));

	return ret;
}


//------------------------------------------------------------------------------------------
//удаляем атрибуты, в которых встречается http://
int zCss::hide_attrs_with_http()
{
	int cnt = 0;

	for(int i = 0; i < (int)m_data.size(); i++)
	{
		zCssItem &item = m_data[i];

		if(item.type == CSS_PARAM)
		{
			string &attr_val = item.s2;
			if(attr_val.find("http://") != string.npos)
			{
				item.hidden = true;
				cnt++;
			}
		}

	}

	return cnt;
}


//------------------------------------------------------------------------------------------
//для add_rand_items
void zCss::_create_rand_items(std::vector<zCssItem> &vec)
{
	//имя
	string name;
	name += (rand() % 2) ? "." : "#";
	name += textutl::get_rand_str(7 + rand() % 5);

	//
	zCssItem item1;
	item1.type = CSS_SELECTOR;
	item1.s1 = name;
	vec.push_back(item1);

	//
	int new_cnt = 1 + rand()%3;
	int used[4] = {0, 0, 0, 0};
	for(int i = 0; i < new_cnt; i++)
	{
		zCssItem item3;
		item3.type = CSS_PARAM;

		int p = 0;
		for(;;)
		{
			p = rand() % 4;
			if(!used[p])
				break;
		}
		item3.s1 = (p == 0) ? "margin" : (p == 1) ? "padding" : (p == 2) ? "border" : "outline";
		used[p]++;

		int val = rand() % 4;
		item3.s2 = zstr::fmt("%d", val);

		vec.push_back(item3);
	}


	//
	zCssItem item9;
	item9.type = CSS_SELECTOR_END;
	vec.push_back(item9);
}


//------------------------------------------------------------------------------------------
//добавляем рандомные item'ы
int zCss::add_rand_items()
{
	int sz = (int)m_data.size();
	if(!sz)
		return 0;

	int dd = sz / 10;
	if(dd < 1)
		dd = 1;
	int cnt = dd + rand() % dd;

	if(cnt < 5)
		cnt = 5;
	if(cnt > sz / 3)
		cnt = sz / 3;
	if(cnt > 50)
		cnt = 50;

	while(cnt > 0)
	{
		int sz2 = (int)m_data.size();

		int ind = rand() % sz2;

		while(ind < sz2 && m_data[ind].type != CSS_SELECTOR_END)
			ind++;
		ind++;
		if(ind <= sz2)
		{
			std::vector<zCssItem> vec;
			_create_rand_items(vec);

			for(int i = 0; i < (int)vec.size(); i++)
				m_data.insert(m_data.begin() + ind++, vec[i]);

			cnt--;
		}
	}

	return cnt;
}
