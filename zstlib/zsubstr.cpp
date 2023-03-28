#include "stdafx.h"

#include "zsubstr.h"


//обрубает с начала и конца пробельные символы " \t\n\r\xA0";
void zsubstr::trim()
{
	//с начала
	while(sz)
	{
		char c = *str;
		if(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\xA0')
		{
			str++;
			sz--;
		}
		else
			break;
	}

	//с конца
	if(sz)
	{
		char *last = str + (sz - 1);
		while(sz)
		{
			char c = *last;
			if(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\xA0')
			{
				last--;
				sz--;
			}
			else
				break;
		}
	}
}


std::string zsubstr::as_string_slow() const
{
	if(!sz)
		return std::string();

	//char prev_c = push_last_zero();
	char prev_c = str[sz];
	str[sz] = 0;

	std::string ret = this->str;

	//pop_prev_char(prev_c);
	str[sz] = prev_c;

	return ret;
}

