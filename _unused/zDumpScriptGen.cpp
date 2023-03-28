#include "stdafx.h"

#include "zDumpScriptGen.h"
#include "zSqliteDbWorkerGB.h"

#include "zDbCashe.h"

using namespace std;


//------------------------------------------------------------------------------------------
zDumpScriptGen::zDumpScriptGen()
{
}


//------------------------------------------------------------------------------------------
zDumpScriptGen::~zDumpScriptGen()
{
}


//------------------------------------------------------------------------------------------
void zDumpScriptGen::generate_Dump(const char *file_name, int first, int last)
{
	int iCheatId1 = first + 4;
	int iCheatId2 = last + 4;

	zSqliteDbWorkerGB &dbWorker = zSqliteDbWorkerGB::inst();
	boost::format fmt("select * from games where ID >= %d and ID <= %d");
	fmt % iCheatId1 % iCheatId2;
	
	dbWorker.exec_some_sql(fmt.str().c_str());
	zDbCashe &cashe = *zSqliteDbWorkerGB::inst().get_db_cashe(Z_TBL_SOME_SQL);

	ofstream file_out(file_name);
	if(!file_out)
		return (_ass(0));

	file_out << "--\n--\n--\n\n\n";
	file_out << "INSERT INTO `games` (`ID`, `Title`, `Desc`, `FullText`, `OurChoice`, `MustPlay`, `MarkInterest`, `MarkSimple`, `MarkQual`, `MarkAll`) VALUES\n";

	for(size_t i = 0; i < cashe.size(); i++)
	{
		boost::format fmt2("(%s, '%s', '%s', '%s', %s, %s, %s, %s, %s, %s)");
		_ass(0);
		/*fmt2 % cashe[i][0];
		fmt2 % cashe[i][2];
		fmt2 % cashe[i][3];
		fmt2 % cashe[i][10];	//FullText
		fmt2 % 0;
		fmt2 % 0;
		fmt2 % cashe[i][5];
		fmt2 % cashe[i][6];
		fmt2 % cashe[i][7];
		fmt2 % cashe[i][8];*/

		string sfmt = fmt2.str();
		string res;
		res.reserve(fmt.size());
		for(size_t j = 0; j < sfmt.size(); j++)
		{
			if(sfmt[j] == '\n')
				res += "\\r\\n";
			else
				res += sfmt[j];
		}

		file_out << res;

		if((i == cashe.size() - 1))
			file_out <<  '.';
		else
			file_out <<  ',';
		file_out << endl;
	}

	file_out.close();
}
