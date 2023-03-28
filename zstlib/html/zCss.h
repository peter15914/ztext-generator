#pragma once


//------------------------------------------------------------------------------------------
enum CSS_ITEM_TYPE
{
	CSS_UNKNOWN,
	CSS_SELECTOR,
	CSS_PARAM,
	CSS_SELECTOR_END
};


//------------------------------------------------------------------------------------------
class zCssItem
{
public:
	std::string s1, s2;
	CSS_ITEM_TYPE type;
	bool hidden;

	zCssItem() : type(CSS_UNKNOWN)
	{
		hidden = false;
	}

	std::string get_print_string();
};


//------------------------------------------------------------------------------------------
class zCss : public boost::noncopyable
{
	std::vector<zCssItem> m_data;

public:
	zCss();
	virtual ~zCss();

	std::vector<zCssItem> &get_data() { return m_data; }

	void add_item(CSS_ITEM_TYPE type, const std::string &s1 = std::string(), const std::string &s2 = std::string());

	//удаляем атрибуты, в которых встречается http://
	int hide_attrs_with_http();

	//добавляем рандомные item'ы
	int add_rand_items();

private:
	//для add_rand_items
	void _create_rand_items(std::vector<zCssItem> &vec);

};

