#include "stdafx.h"

#include "zContentGenServ.h"
#include "zSqliteDbWorkerGB.h"
#include "zDbCashe.h"


using namespace std;


//------------------------------------------------------------------------------------------
zContentGenServ::zContentGenServ()
{
}


//------------------------------------------------------------------------------------------
zContentGenServ::~zContentGenServ()
{
}


//------------------------------------------------------------------------------------------
void zContentGenServ::gen_SeoTexts(const char *sFileName, int id)
{
	int iCheatId = id + 4;

	zSqliteDbWorkerGB &dbWorker = zSqliteDbWorkerGB::inst();

	string sql = (boost::format("select * from games where ID=%d") % iCheatId).str();
	dbWorker.exec_some_sql(sql.c_str());

	if(open_db())
		return _ass(0);

	//читаем файл
	string text;

	ifstream file(sFileName);
	string line;
	while(file && getline(file, line))
	{
		_replace_Keywords(line);
		text += line + "\n";
	}

	file.close();

	//апдейтим запись
	boost::format fmt("UPDATE games SET seoText01 = '%s' WHERE ID = %d");
	fmt % text % iCheatId;

	int rc = exec(fmt.str(), 0);
	if(rc)
		(_ass(0));


	close_db();
}


//------------------------------------------------------------------------------------------
void zContentGenServ::_replace_Keywords(std::string &str)
{
	string ret;

	zDbCashe &cashe = *zSqliteDbWorkerGB::inst().get_db_cashe(Z_TBL_SOME_SQL);
	if(cashe.size() != 1)
		return (_ass(0));

	size_t prev = 0;
	while(1)
	{
		size_t jj = str.find("{$", prev);
		if(jj == string.npos)
			break;

		size_t jj2 = str.find("}", jj);
		if(jj2 == string.npos)
			break;

		ret += str.substr(prev, jj-prev);

		//
		//int iColNum = atoi(str.substr(jj+2, jj2-jj-2).c_str());
		string fieldName = str.substr(jj+2, jj2-jj-2).c_str();

		/*if(iColNum < 0 || iColNum >= (int)cashe[0].size())
			(_ass(0));
		else*/
			ret += cashe[0][fieldName];

		prev = jj2 + 1;
	}

	if(prev < str.length())
		ret += str.substr(prev, str.length());

	str = ret;
}


//------------------------------------------------------------------------------------------
//вырезаем мусор, заменяем символы типа &nbsp;
void zContentGenServ::gen_FullText2(int id)
{
	int iCheatId = id + 4;

	zSqliteDbWorkerGB &dbWorker = zSqliteDbWorkerGB::inst();

	string sql = (boost::format("select * from games where ID=%d") % iCheatId).str();
	dbWorker.exec_some_sql(sql.c_str());

	zDbCashe &cashe = *zSqliteDbWorkerGB::inst().get_db_cashe(Z_TBL_SOME_SQL);
	if(cashe.size() != 1)
		return (_ass(0));

	//читаем существующий FullText
	string str = cashe[0]["FullText"];
	_replace_Garbage(str);

	//
	if(open_db())
		return _ass(0);

	//апдейтим запись
	boost::format fmt("UPDATE games SET FullText2 = '%s' WHERE ID = %d");
	fmt % str % iCheatId;

	int rc = exec(fmt.str(), 0);
	if(rc)
		(_ass(0));


	close_db();
}


//------------------------------------------------------------------------------------------
//вырезаем мусор, заменяем символы типа &nbsp;
void zContentGenServ::_replace_Garbage(std::string &str)
{
	string ret;
	ret.reserve(str.length());

	for(size_t i = 0; i < str.length(); i++)
	{
		char c = str[i];
		if(c == '<')
		{
			size_t jj = str.find('>', i+1);
			if(jj == str.npos)
				_ass(0);
			else
			{
				i = jj;
				continue;
			}
		}
		else
		if(c == '&')
		{
			size_t jj = str.find(';', i+1);
			if(jj == str.npos)
				_ass(0);
			else
			{
				string test = str.substr(i, jj-i+1);
				_ass(jj-i <= 7);
				i = jj;
				continue;
			}
		}

		ret += c;
	}

	str = ret;
}

