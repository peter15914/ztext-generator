#pragma once


struct sqlite3;
class zDbCashe;


//------------------------------------------------------------------------------------------
enum Z_DB_TABLE
{
	Z_TBL_NONE,
	Z_TBL_GAMES,
	Z_TBL_GENRES,
	Z_TBL_REL_GG,
	Z_TBL_SOME_SQL
};


//------------------------------------------------------------------------------------------
class zSqliteDbWorkerGB
{
	sqlite3 *m_db;
	char *m_sErrMsg;

public:
	zSqliteDbWorkerGB();
	~zSqliteDbWorkerGB();

	static zSqliteDbWorkerGB &inst();

	zDbCashe *get_db_cashe(Z_DB_TABLE tbl);
	int get_count(Z_DB_TABLE tbl);

	bool exec_some_sql(const char *statement);

protected:
	bool _open_db();
	bool _close_db();

	void _free_err_str();

	void _load_tables();
	bool _exec_sql(const char *statement);
};
