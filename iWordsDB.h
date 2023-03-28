#pragma once


//------------------------------------------------------------------------------------------
class iWordsDB
{
public:
	virtual bool is_valid_id(int id) = 0;
	virtual const char *get_str_by_id(int id) = 0;
};
