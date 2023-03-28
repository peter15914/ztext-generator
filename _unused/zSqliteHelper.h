#pragma once

struct sqlite3;

class zSqliteHelper
{
	sqlite3 *m_db;
	char *m_sErrMsg;

public:
	zSqliteHelper();
	virtual ~zSqliteHelper();

	//возвращает return code sqlite
	int open_db();
	void close_db();

	sqlite3 *get_db();

	//sQuery - SQL to be evaluated
	//callback - Callback function
	int exec(const std::string &sQuery, int (*callback)(void*,int,char**,char**));

protected:

};

