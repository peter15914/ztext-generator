#pragma once

#include <zstlib/sqlite/zDbCashe.h>

struct sqlite3;


//------------------------------------------------------------------------------------------
class zSqliteHelper
{
	sqlite3 *m_db;
	char *m_sErrMsg;

	//код, возвращенный последним вызовом exec-функций
	//для проверки сразу же после вызова
	int m_last_rc;

	//последний выполняемый запрос
	std::string m_last_sql;

	//для хранения разультатов после работы функций exec3
	zDbCashe m_res_dbc;

public:
	zSqliteHelper();
	zSqliteHelper(const char *db_file_name);
	virtual ~zSqliteHelper();

	//возвращает return code sqlite
	int open_db(const char *db_file_name);
	void close_db();

	sqlite3 *get_sqlite3_db();
	bool is_opened() { return m_db != 0; }

	//код, возвращенный последним вызовом exec-функций
	//для проверки сразу же после вызова
	int rc() { return m_last_rc; }

	//для хранения разультатов после работы функций exec3
	zDbCashe &dbc() { return m_res_dbc; };

	//sQuery - SQL to be evaluated
	//callback - Callback function
	//возвращает return-code от sqlite: SQLITE_OK - если всё нормально
	int exec(const char *statement, int (*callback)(void*,int,char**,char**) = 0);
	int exec(const std::string &statement, int (*callback)(void*,int,char**,char**) = 0);
	int exec(boost::format &statement, int (*callback)(void*,int,char**,char**) = 0);

	//выполняет sql и кладет результат в res_dbc
	//возвращает return-code от sqlite: SQLITE_OK - если всё нормально
	int exec2(const char *statement, zDbCashe *res_dbc);
	int exec2(const std::string &statement, zDbCashe *res_dbc);
	int exec2(boost::format &statement, zDbCashe *res_dbc);

	//выполняет sql и кладет результат в res_dbc
	zDbCashe &exec3(const char *statement);
	zDbCashe &exec3(const std::string &statement);
	zDbCashe &exec3(boost::format &statement);


	//старые длянные названия, оставлены для совместимости
	int exec_sql_to_cache(const char *statement, zDbCashe *res_dbc)
	{
		return exec2(statement, res_dbc);
	}
	int exec_sql_to_cache(const std::string &statement, zDbCashe *res_dbc)
	{
		return exec2(statement, res_dbc);
	}
	
	//return true if ok
	bool get_tables(std::vector<std::string> &tables);

protected:

};

