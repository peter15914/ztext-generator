#include "stdafx.h"
#include "zParseUtl.h"

#include "zDbCashe.h"
#include "zSqliteDbWorkerGB.h"

using namespace std;

#define BREAK_IF_NO(x) if((x) == string.npos) {_ass(0); break;};
#define BREAK_IF_NO_WITHOUT_ASS(x) if((x) == string.npos) break;
#define CONTINUE_IF_NO(x) if((x) == string.npos) continue;

#define RET_IF_NO(x) if((x) == string.npos) {_ass(0); return;};
#define RET_IF_NO_WITHOUT_ASS(x) if((x) == string.npos) return;

//------------------------------------------------------------------------------------------
zParseUtl::zParseUtl()
{
}


//------------------------------------------------------------------------------------------
zParseUtl::~zParseUtl()
{
}


//------------------------------------------------------------------------------------------
//ищем строки такого вида: <li><....><a title="Игра Блеск, прохождению" href="/review/game-Zeal/">Блеск</a></li>
void zParseUtl::_parse_LineGamesList(const string &line, ofstream &file_out)
{
	size_t i = 0;
	while(i < line.length())
	{
		size_t j = line.find("<li>", i); BREAK_IF_NO(j);
		j += 4;

		size_t j2 = line.find("</li>", j); BREAK_IF_NO(j2);
		i = j2 + 4;

		string s2 = line.substr(j, j2-j);

		size_t a1 = s2.find("<a", 0); CONTINUE_IF_NO(a1);
		size_t a2 = s2.find("</a>", a1+1); CONTINUE_IF_NO(a2);

		size_t t = s2.find("title=\"", a1+1); CONTINUE_IF_NO(t);
		size_t h = s2.find("href=\"", t+1); CONTINUE_IF_NO(h);

		if(!(a2 < j2 && h < a2))
			continue;

		size_t q1 = s2.find(L'\"', t+1); CONTINUE_IF_NO(q1);
		size_t q2 = s2.find(L'\"', q1+1); CONTINUE_IF_NO(q2);

		size_t q3 = s2.find(L'\"', h+1); CONTINUE_IF_NO(q3);
		size_t q4 = s2.find(L'\"', q3+1); CONTINUE_IF_NO(q4);

		size_t b1 = s2.find(L'>', q4+1); CONTINUE_IF_NO(b1);

		//file_out << s2 << endl;

		string sDescr = s2.substr(q1+1, q2-q1-1);
		string sHref = s2.substr(q3+1, q4-q3-1);
		string sTitle = s2.substr(b1+1, a2-b1-1);
		//file_out << sDescr << endl;
		//file_out << sHref << endl;
		//file_out << sTitle << endl;
		//file_out << endl;

		int iNevoId = 0;

		boost::format fmt("insert into games values(null,%d,'%s','%s','%s',0,0,0,0,'%s')");
		fmt % iNevoId;
		fmt % sTitle;
		fmt % sDescr;
		fmt % "";
		fmt % sHref;

		int rc = exec(fmt.str(), 0);
		_ass(!rc);
	}

}


//------------------------------------------------------------------------------------------
//проверяем, вдруг уже есть эта таблица
//если нет нужной таблицы, то создаем ее
void zParseUtl::_crate_TableIfNeed(std::string sTable, std::string sCols)
{
	static int ret_argc = 0;
	//проверяем, вдруг уже есть эта таблица
	struct zzz {
		static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
			ret_argc = argc;
			return 0;
		}
	};

	string sQuery = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + sTable + "';";
	int rc = exec(sQuery, zzz::callback);
	if(rc)
	{
		close_db();
		return _ass(0);
	}

	if(ret_argc == 0)
	{
		//если нет нужной таблицы, то создаем ее
		sQuery = "create table " + sTable + "(" + sCols + ");";
		rc = exec(sQuery, 0);
		if(rc)
		{
			close_db();
			return _ass(0);
		}
	}
}


//------------------------------------------------------------------------------------------
void zParseUtl::parse_GamesList(string sFile)
{
	//setlocale(LC_CTYPE, ".ACP");

	if(open_db())
		return _ass(0);

	//_crate_TableIfNeed("games", "id integer primary key, title text, href text");

	//читаем файл
	ifstream file(sFile.c_str());

	string line, res;
	while(file && getline(file, line))
		res += line;

	//парсим
	ofstream file_out((sFile + ".parse").c_str());
	_parse_LineGamesList(res, file_out);

	//очищаем всё и закрываем
	file.close();
	file_out.close();
	close_db();
}


//------------------------------------------------------------------------------------------
void zParseUtl::_parse_GameReview(const string &line, int id, ofstream &file_out)
{
	if(open_db())
		return _ass(0);

	//size_t i = 0;
	string sAvatarImg, sSize, sAvatar2Img, sDescr, sNevoId;
	vector<string> aScreens, aThumbs;
	while(1)//i < line.length())
	{
		size_t j = line.find("<h1>"); BREAK_IF_NO(j);
		j += 4;
		size_t i1 = line.find("<img class=\"Avatar\"", j); BREAK_IF_NO(i1);

		size_t q1 = line.find("src=\"", i1); BREAK_IF_NO(q1);
		q1 += 5;
		size_t q2 = line.find('\"', q1); BREAK_IF_NO(q2);

		sAvatarImg = line.substr(q1, q2-q1);

		size_t d1 = line.find("\"downloadSize\">", q2);
		if(d1 != string.npos)
		{
			size_t b1 = line.find('(', d1); BREAK_IF_NO(b1);
			size_t b2 = line.find(')', b1); BREAK_IF_NO(b2);
			sSize = line.substr(b1+1, b2-b1-1);
		}
		else
		{
			sSize = "";
			d1 = q2;
		}

		size_t i2 = line.find("<img class=\"Avatar\"", d1); BREAK_IF_NO(i2);

		size_t q3 = line.find("src=\"", i2); BREAK_IF_NO(q3);
		q3 += 5;
		size_t q4 = line.find('\"', q3); BREAK_IF_NO(q4);

		sAvatar2Img = line.substr(q3, q4-q3);

		//скрины должны идти до rf
		size_t rf = line.find("right_field", 0); BREAK_IF_NO(q4);
		//ищем скрины
		size_t jj = q4;
		while(jj < rf)
		{
			size_t g1 = line.find("target=\"_blank\"", jj);
			if(g1 > rf)
				break;

			size_t q7 = line.find("href=\"", g1); BREAK_IF_NO(q7);
			q7 += 6;
			size_t q8 = line.find('\"', q7); BREAK_IF_NO(q8);
			
			aScreens.push_back(line.substr(q7, q8-q7));

			size_t q5 = line.find("src=\"", g1); BREAK_IF_NO(q5);
			q5 += 5;
			size_t q6 = line.find('\"', q5); BREAK_IF_NO(q6);
			jj = q6 + 1;
			
			aThumbs.push_back(line.substr(q5, q6-q5));
		}

		////std::string sBegTag = "</script><br class=\"divide\" />";

		////size_t p1 = line.find("<p>", jj); BREAK_IF_NO(p1);
		////p1 += 3;
		//size_t pp1 = line.find(sBegTag, jj); BREAK_IF_NO(pp1);
		//pp1 += sBegTag.size();
		////size_t dd = line.find("</div>", p1); BREAK_IF_NO(dd);

		////мы должны найти последний </p> перед dd
		////size_t p2 = line.find("</p>", p1); BREAK_IF_NO(p2);
		//size_t pp2 = line.find("</div>", pp1); BREAK_IF_NO(pp2);
		///*while(1)
		//{
		//	size_t p2New = line.find("</p>", p2+1);
		//	if(p2New == string.npos || p2New > dd)
		//		break;
		//	p2 = p2New;
		//}*/

		////sDescr = line.substr(p1, p2-p1);
		//sDescr = line.substr(pp1, pp2-pp1);

		size_t pp10 = line.find("addthis.addEventListener", 0); BREAK_IF_NO(pp10);
		size_t pp12 = line.find("</div>", pp10); BREAK_IF_NO(pp12);

		//если до </div> есть <p>, то это нормальный вариант
		size_t pp11 = line.find("<p>", pp10);
		size_t pp11_2 = line.find("<p ", pp10);

		if(pp11 != string.npos && pp11 < pp12)
			sDescr = line.substr(pp11, pp12-pp11);
		else
		if(pp11_2 != string.npos && pp11_2 < pp12)
			sDescr = line.substr(pp11_2, pp12-pp11_2);
		else
		{
			//пропускаем все </script> до </div>
			while(1)
			{
				size_t zz01 = line.find("</script>", pp10);BREAK_IF_NO_WITHOUT_ASS(zz01);
				zz01 += 9;
				if(zz01 < pp12)
					pp10 = zz01;
				else
					break;
			}

			size_t pp11 = line.find("<br class=\"divide\" />", pp10);
			if(pp11 != string.npos && pp11 < pp12)
			{
				pp11 += string("<br class=\"divide\" />").length();
				sDescr = line.substr(pp11, pp12-pp11);
			}
			else
			{
				_ass(0);
				sDescr = line.substr(pp10, pp12-pp10);
			}
		}		

		size_t ss = line.find("sms/id-", 0);
		if(ss != string.npos)
		{
			ss += 7;
			size_t q9 = line.find('\"', ss); BREAK_IF_NO_WITHOUT_ASS(q9);

			sNevoId = line.substr(ss, q9 - ss);
		}

		//также номер игры выдираем из скринов
		size_t bb01 = sAvatarImg.find_last_of('/');
		BREAK_IF_NO(bb01);
		size_t bb02 = sAvatarImg.find_last_of('/', bb01-1);
		BREAK_IF_NO(bb02);
		string sssTempId = sAvatarImg.substr(bb02+1, bb01-bb02-1);

		if(sNevoId.empty())
			sNevoId = sssTempId;
		else
		{
			if(atoi(sssTempId.c_str()) != atoi(sNevoId.c_str()))
				_ass(0);
		}

		break;
	}

	file_out << sAvatarImg << endl;
	file_out << sSize << endl;
	file_out << sAvatar2Img << endl;

	file_out << "Thumbs:" << endl;
	for(size_t i = 0; i < aThumbs.size(); i++)
		file_out << aThumbs[i] << endl;

	file_out << "Screens:" << endl;
	for(size_t i = 0; i < aScreens.size(); i++)
		file_out << aScreens[i] << endl;

	file_out << sDescr << endl;
	file_out << sNevoId << endl;


	if(sNevoId.empty())
		return (_ass(0));
	int iNevoId = atoi(sNevoId.c_str());

	if(iNevoId == 0)
		return (_ass(0));


	/*
	//----------------------------
	//апдейтим FullText:
	int iCheatId = id + 4;
	//UPDATE names SET name = "John" WHERE id = 1
	std::string sQuery = (boost::format("UPDATE games SET FullText = '%s' WHERE ID = %d") % sDescr % iCheatId).str();
	int rc = exec(sQuery, 0);
	if(rc)
	{
		close_db();
		return _ass(0);
	}//*/
	//----------------------------

	//*
	//----------------------------
	//апдейтим nevID:
	int iCheatId = id + 4;
	//UPDATE names SET name = "John" WHERE id = 1
	std::string sQuery = (boost::format("UPDATE games SET nevID = %d WHERE ID = %d") % iNevoId % iCheatId).str();
	int rc = exec(sQuery, 0);
	if(rc)
	{
		close_db();
		return _ass(0);
	}//*/
	//----------------------------

}


//------------------------------------------------------------------------------------------
//парсит ревью игры
void zParseUtl::parse_GameReview(int id)
{
	string sFile = (boost::format("review/%03d.txt") % id).str();

	ifstream file(sFile.c_str());
	if(!file)
		return;// (_ass(0));

	string line, res;
	while(file && getline(file, line))
		res += line;

	//парсим
	ofstream file_out((sFile + ".parse").c_str());
	_parse_GameReview(res, id, file_out);

	//очищаем всё и закрываем
	file.close();
	file_out.close();
}


//------------------------------------------------------------------------------------------
//парсит один конкретный жанр
void zParseUtl::parse_Genre(int iGenreId, std::string sFile)
{
	ifstream file(sFile.c_str());
	if(!file)
		return (_ass(0));

	string line, res;
	while(file && getline(file, line))
		res += line;

	//парсим
	ofstream file_out((sFile + ".parse").c_str());
	_parse_Genre(res, iGenreId, file_out);

	//очищаем всё и закрываем
	file.close();
	file_out.close();
}


//------------------------------------------------------------------------------------------
void zParseUtl::_parse_Genre(const std::string &line, int iGenreId, std::ofstream &file_out)
{
	if(open_db())
		return _ass(0);

	zSqliteDbWorkerGB &dbWorker = zSqliteDbWorkerGB::inst();


	size_t j01 = line.find("counter.reCount();"); RET_IF_NO(j01);
	size_t j02 = line.find("</h2>", j01); RET_IF_NO(j02);

	j02 = j02 + string("</h2></div></div></div>").length();
	size_t jLast  = line.find("</div>", j02); RET_IF_NO(jLast);

	size_t cur = j02;
	while(cur < jLast)
	{
		size_t j10 = line.find("href=\"/review/", cur); RET_IF_NO_WITHOUT_ASS(j10);
		size_t j11 = line.find("/\">", j10); RET_IF_NO(j11);

		j10 += 6;
		string href = line.substr(j10, j11-j10+1);
		file_out << href << std::endl;

		//ищем игру с таким href
		string sql = (boost::format("select * from games where Href='%s'") % href).str();
		dbWorker.exec_some_sql(sql.c_str());

		zDbCashe *cashe = dbWorker.get_db_cashe(Z_TBL_SOME_SQL);
		if(cashe->size() != 1)
			_ass(0);
		else
		{
			int ii = 0;
		}

		cur = j11;
	}

}

