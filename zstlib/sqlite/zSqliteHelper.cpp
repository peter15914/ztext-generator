#include "stdafx.h"

#include "zSqliteHelper.h"


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace std;


//------------------------------------------------------------------------------------------
zSqliteHelper::zSqliteHelper() :
	m_db(0),
	m_sErrMsg(0),
	m_last_rc(INT_MAX)
{
}


//------------------------------------------------------------------------------------------
zSqliteHelper::zSqliteHelper(const char *db_file_name) :
	m_db(0),
	m_sErrMsg(0),
	m_last_rc(INT_MAX)
{
	open_db(db_file_name);
}


//------------------------------------------------------------------------------------------
zSqliteHelper::~zSqliteHelper()
{
	if(m_db)
		sqlite3_close(m_db);
	m_db = 0;

	if(m_sErrMsg)
		sqlite3_free(m_sErrMsg);
	m_sErrMsg = 0;
}


//------------------------------------------------------------------------------------------
sqlite3 *zSqliteHelper::get_sqlite3_db()
{
	_ass(m_db);
	return m_db;
}


//------------------------------------------------------------------------------------------
//возвращает return code sqlite
int zSqliteHelper::open_db(const char *db_file_name)
{
	int rc = sqlite3_open(db_file_name, &m_db);
	if(rc)
	{
		sqlite3_close(m_db);
		m_db = 0;
		_ass(0);
	}
	return rc;
}


//------------------------------------------------------------------------------------------
void zSqliteHelper::close_db()
{
	sqlite3_close(m_db);
	m_db = 0;
}


//------------------------------------------------------------------------------------------
//sQuery - SQL to be evaluated
//callback - Callback function
//возвращает return-code от sqlite: SQLITE_OK - если всё нормально
int zSqliteHelper::exec(const char *statement, int (*callback)(void*,int,char**,char**)/* = 0*/)
{
	m_last_sql = statement;

	int rc = sqlite3_exec(m_db, statement, callback, 0, &m_sErrMsg);
	_ass(rc == SQLITE_OK);
	m_last_rc = rc;
	return rc;
}


//------------------------------------------------------------------------------------------
int zSqliteHelper::exec(const std::string &statement, int (*callback)(void*,int,char**,char**)/* = 0*/)
{
	return exec(statement.c_str(), callback);
}


//------------------------------------------------------------------------------------------
int zSqliteHelper::exec(boost::format &statement, int (*callback)(void*,int,char**,char**)/* = 0*/)
{
	return exec(statement.str().c_str(), callback);
}


//------------------------------------------------------------------------------------------
//для exec2
zDbCashe *g_CurCashe = 0;
static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	if(!g_CurCashe)
		return (_ass(0), 0);

	//колонки добавляем только 1 раз, а остальные разы - верим, что всё нормально
	if(g_CurCashe->empty())
	{
		for(int i = 0 ; i < argc; i++)
			g_CurCashe->add_column(i, azColName[i] ? azColName[i] : std::string());
	}

	//добавляем row
	int vec_ind = g_CurCashe->add_row();

	for(int i = 0 ; i < argc; i++)
		g_CurCashe->set_value_on_fill(vec_ind, i, argv[i]);

	return 0;
}


//------------------------------------------------------------------------------------------
//выполняет sql и кладет результат в res_dbc
//возвращает return-code от sqlite: SQLITE_OK - если всё нормально
int zSqliteHelper::exec2(const char *statement, zDbCashe *res_dbc)
{
	if(!res_dbc || !statement)
		return (_ass(0), m_last_rc = SQLITE_ERROR);

	m_last_sql = statement;

	res_dbc->clear();
	res_dbc->clear_columns();

	g_CurCashe = res_dbc;

	int rc = 0;
	if(1)
	{
		//нормальная версия
		rc = sqlite3_exec(m_db, statement, callback, 0, &m_sErrMsg);
	}
	else
	{
		//тестовая версия
		for(int ii = 0; ii < 10; ii++)
			rc = sqlite3_exec(m_db, statement, callback, 0, &m_sErrMsg);
	}

	g_CurCashe = 0;

	if(rc != SQLITE_OK)
	{
		string err = zstr::fmt("SQL error: %s", m_sErrMsg);
		zdebug::log()->LogImp(err);
		_ass(0);
		res_dbc->clear();
	}

	if(m_sErrMsg)
		sqlite3_free(m_sErrMsg);

	_ass(rc == SQLITE_OK);
	m_last_rc = rc;
	return rc;
}


//------------------------------------------------------------------------------------------
int zSqliteHelper::exec2(const std::string &statement, zDbCashe *res_dbc)
{
	return exec2(statement.c_str(), res_dbc);
}


//------------------------------------------------------------------------------------------
int zSqliteHelper::exec2(boost::format &statement, zDbCashe *res_dbc)
{
	return exec2(statement.str().c_str(), res_dbc);
}


//------------------------------------------------------------------------------------------
zDbCashe &zSqliteHelper::exec3(const char *statement)
{
	exec2(statement, &m_res_dbc);
	return m_res_dbc;
}


//------------------------------------------------------------------------------------------
zDbCashe &zSqliteHelper::exec3(const std::string &statement)
{
	exec2(statement.c_str(), &m_res_dbc);
	return m_res_dbc;
}


//------------------------------------------------------------------------------------------
zDbCashe &zSqliteHelper::exec3(boost::format &statement)
{
	exec2(statement.str().c_str(), &m_res_dbc);
	return m_res_dbc;
}


//------------------------------------------------------------------------------------------
//return true if ok
bool zSqliteHelper::get_tables(std::vector<std::string> &tables)
{
	tables.clear();

	zDbCashe buf_dbc;
	exec2("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;", &buf_dbc);

	int tbl_cnt = buf_dbc.size();
	for(int i = 0; i < tbl_cnt; i++)
	{
		zDataRow& row = buf_dbc[i];
		zDataItem itm = row.get_item(0);

		//это служебная таблица
		if(!strcmp("sqlite_sequence", itm.m_val))
			continue;

		tables.push_back(string(itm.m_val));
	}

	return true;
}

