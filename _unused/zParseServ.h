#pragma once



//------------------------------------------------------------------------------------------
class zParseServ
{
	zParseServ();

public:
	virtual ~zParseServ();

	static zParseServ& get_ParseServ();

	enum LGP_TYPE
	{
		LGP_NONE,
		LGP_REVIEW
	};
	//загружает все страницы для игры, id из таблицы games
	bool load_GamePages(int id, LGP_TYPE type);
};

