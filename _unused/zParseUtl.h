#pragma once

#include "zSqliteHelper.h"


//------------------------------------------------------------------------------------------
class zParseUtl : public zSqliteHelper
{
public:
	zParseUtl();
	virtual ~zParseUtl();

	//http://www.test.ru/genres/
	void parse_GamesList(std::string sFile);

	//парсит ревью игры
	void parse_GameReview(int id);

	//парсит один конкретный жанр
	void parse_Genre(int iGenreId, std::string sFile);

protected:
	void _parse_LineGamesList(const std::string &line, std::ofstream &file_out);

	void _parse_GameReview(const std::string &line, int id, std::ofstream &file_out);

	void _parse_Genre(const std::string &line, int iGenreId, std::ofstream &file_out);

	//проверяем, вдруг уже есть эта таблица
	//если нет нужной таблицы, то создаем ее
	void _crate_TableIfNeed(std::string sTable, std::string sCols);

	//int _get_NevIdFromID(int ID);
};

