#pragma once

#include "zSqliteHelper.h"


//------------------------------------------------------------------------------------------
class zDumpScriptGen : public zSqliteHelper
{
public:
	zDumpScriptGen();
	virtual ~zDumpScriptGen();

	void generate_Dump(const char *file_name, int first, int last);
};
