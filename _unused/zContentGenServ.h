#pragma once

#include "zSqliteHelper.h"


//------------------------------------------------------------------------------------------
class zContentGenServ : public zSqliteHelper
{

public:
	zContentGenServ();
	virtual ~zContentGenServ();

	void gen_SeoTexts(const char *sFileName, int id);

	//вырезаем мусор, заменяем символы типа &nbsp;
	void gen_FullText2(int id);

private:

	void _replace_Keywords(std::string &str);

	//вырезаем мусор, заменяем символы типа &nbsp;
	void _replace_Garbage(std::string &str);
};

