#include "stdafx.h"

#include "zSqliteDbWorkerGB.h"
#include "zDbCashe.h"

namespace zzz
{
	zDbCashe *g_CurCashe;

	zDbCashe g_CasheGames;
	zDbCashe g_CasheGenres;
	zDbCashe g_CasheRelGamesGenres;

	zDbCashe g_CasheSomeSql;
}

using namespace zzz;

zSqliteDbWorkerGB::zSqliteDbWorkerGB() :
	m_sErrMsg(0),
	m_db(0)
{
	_open_db();
	_load_tables();
}


zSqliteDbWorkerGB::~zSqliteDbWorkerGB()
{
	_close_db();
	_free_err_str();
}


zSqliteDbWorkerGB &zSqliteDbWorkerGB::inst()
{
	static zSqliteDbWorkerGB SqliteDbWorkerGB;
	return SqliteDbWorkerGB;
}


bool zSqliteDbWorkerGB::_open_db()
{
	int rc = sqlite3_open(zconst::DB_NAME.c_str(), &m_db);
	if(rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_db));
		sqlite3_close(m_db);
		return false;
	}

    return true;
}

bool zSqliteDbWorkerGB::_close_db()
{
	sqlite3_close(m_db);
	return true;
}


void zSqliteDbWorkerGB::_free_err_str()
{
	if(m_sErrMsg)
		sqlite3_free(m_sErrMsg);
	m_sErrMsg = 0;
}


void zSqliteDbWorkerGB::_load_tables()
{
	if(!g_CasheGames.empty() || !g_CasheGenres.empty() || !g_CasheRelGamesGenres.empty())
		return (_ass(0));

	g_CurCashe = &g_CasheGames;
	_exec_sql("select * from games;");

	g_CurCashe = &g_CasheGenres;
	_exec_sql("select * from genres;");

	g_CurCashe = &g_CasheRelGamesGenres;
	_exec_sql("select * from RelGamesGenres;");

	g_CurCashe = 0;
}


static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	/*char aaa[2000];
	for(int i=0; i<argc; i++)
	{
		sprintf(aaa, "%s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
		int ii = 0;
	}*/

	size_t sz = g_CurCashe->size();
	g_CurCashe->resize(sz + 1);

	row_cashe &vec = (*g_CurCashe)[sz];
	vec.set_parent(g_CurCashe);

	for(int i = 0 ; i < argc; i++)
		vec.push_back(argv[i] ? argv[i] : std::string());

	for(int i = 0 ; i < argc; i++)
		g_CurCashe->add_column(azColName[i] ? azColName[i] : std::string(), i);

	return 0;
}


bool zSqliteDbWorkerGB::_exec_sql(const char *statement)
{
	g_CurCashe->clear_columns();

	int rc = sqlite3_exec(m_db, statement, callback, 0, &m_sErrMsg);
	if(rc != SQLITE_OK)
		fprintf(stderr, "SQL error: %s\n", m_sErrMsg);

	_free_err_str();

	return (rc == SQLITE_OK);
}


zDbCashe *zSqliteDbWorkerGB::get_db_cashe(Z_DB_TABLE tbl)
{
	zDbCashe *ret = 0;

	switch(tbl)
	{
		case Z_TBL_GAMES:
			ret = &g_CasheGames;
		break;

		case Z_TBL_GENRES:
			ret = &g_CasheGenres;
		break;

		case Z_TBL_REL_GG:
			ret = &g_CasheRelGamesGenres;
		break;

		case Z_TBL_SOME_SQL:
			ret = &g_CasheSomeSql;
		break;

		default:
			_ass(0);
	}

	return ret;
}


int zSqliteDbWorkerGB::get_count(Z_DB_TABLE tbl)
{
	zDbCashe *cashe = get_db_cashe(tbl);
	return cashe ? (int)cashe->size() : 0;
}


bool zSqliteDbWorkerGB::exec_some_sql(const char *statement)
{
	g_CasheSomeSql.resize(0);

	g_CurCashe = &g_CasheSomeSql;
	bool ret = _exec_sql(statement);

	return ret;
}

