#include "stdafx.h"

#include "zSqliteHelper.h"




//------------------------------------------------------------------------------------------
zSqliteHelper::zSqliteHelper() :
	m_db(0),
	m_sErrMsg(0)
{
}


//------------------------------------------------------------------------------------------
zSqliteHelper::~zSqliteHelper()
{
	if(m_sErrMsg)
		sqlite3_free(m_sErrMsg);
	m_sErrMsg = 0;
}


//------------------------------------------------------------------------------------------
sqlite3 *zSqliteHelper::get_db()
{
	_ass(m_db);
	return m_db;
}


//------------------------------------------------------------------------------------------
//возвращает return code sqlite
int zSqliteHelper::open_db()
{
	int rc = sqlite3_open(zconst::DB_NAME.c_str(), &m_db);
	if(rc)
	{
		sqlite3_close(m_db);
		_ass(0);
	}
	return rc;
}


//------------------------------------------------------------------------------------------
void zSqliteHelper::close_db()
{
	sqlite3_close(m_db);
}


//------------------------------------------------------------------------------------------
//sQuery - SQL to be evaluated
//callback - Callback function
int zSqliteHelper::exec(const std::string &sQuery, int (*callback)(void*,int,char**,char**))
{
	int rc = sqlite3_exec(m_db, sQuery.c_str(), callback, 0, &m_sErrMsg);
	return rc;
}

