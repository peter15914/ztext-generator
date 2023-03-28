#include "stdafx.h"
#include "zParseServ.h"

#include "zPageDownloader.h"


using namespace std;


//------------------------------------------------------------------------------------------
zParseServ::zParseServ()
{
}


//------------------------------------------------------------------------------------------
zParseServ::~zParseServ()
{
}


//------------------------------------------------------------------------------------------
zParseServ& zParseServ::get_ParseServ()
{
	static zParseServ z;
	return z;
}


//------------------------------------------------------------------------------------------
//загружает все страницы для игры, id из таблицы games
bool zParseServ::load_GamePages(int id, LGP_TYPE type)
{
	sqlite3 *db = 0;

	int rc = sqlite3_open_v2(zconst::DB_NAME.c_str(), &db, SQLITE_OPEN_READONLY, 0);
	if(rc)
	{
		sqlite3_close(db);
		return (_ass(0), true);
	}

	ostringstream ost;
	ost << "select href from games where id = " << id << ";";
	ost.flush();
	string sQuery = ost.str();

	sqlite3_stmt *pStmt;
	int ret = sqlite3_prepare_v2(db, sQuery.c_str(), sQuery.length(), &pStmt, 0);

	string sHref;
	if(ret == SQLITE_OK)
	{
		while(1)
		{
			int bufRet = sqlite3_step(pStmt);
			if(bufRet == SQLITE_ERROR || bufRet == SQLITE_MISUSE || bufRet == SQLITE_DONE)
				break;

			if(bufRet == SQLITE_ROW)
			{
				int type = sqlite3_column_type(pStmt, 0);
				_ass(type == SQLITE_TEXT);
				const unsigned char *href = sqlite3_column_text(pStmt, 0);
				sHref = reinterpret_cast<const char*>(href);
				break;
			}
		}
	}
	sqlite3_finalize(pStmt);
	sqlite3_close(db);

	//в sHref у нас содержится link на страницы с игрой в виде /review/game-7-Lands/
	if(type == LGP_REVIEW)
	{
		char fileName[200];
		sprintf(fileName, "review/%03d.txt", id);

		zPageDownloader ldr;

		string sUrl = "http://www.test.ru" + sHref;
		sUrl.erase(sUrl.length()-1);
		ldr.get_UrlToFile(sUrl, fileName);

	}

	return true;
}

