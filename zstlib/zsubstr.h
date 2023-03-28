#pragma once


struct zsubstr
{
	char *str;
	int sz;

	__forceinline void init()
	{
		//memset(this, 0, sizeof(zsubstr));
		str = 0;
		sz = 0;
	}

	__forceinline void init_empty()
	{
		static char a[1] = {0};
		str = &a[0];
		sz = 0;
	}

	__forceinline void clear()
	{
		//str = 0;
		sz = 0;
	}

	__forceinline void set(char *_str, int _sz)
	{
		str = _str;
		sz = _sz;
	}

	//[ _beg, _end )
	__forceinline void set(char *_str, int _beg, int _end)
	{
		str = _str + _beg;
		sz = _end - _beg;
	}

	__forceinline bool empty() const
	{
		return !sz;
	}

	__forceinline int size() const
	{
		return sz;
	}

	__forceinline bool equal_(const char *str2) const
	{
		return !memcmp(str2, str, sz) && (str2[sz] == 0);	//чтоб не просто проверка на подстроку была
	}

	//без учета регистра
	__forceinline bool equal_nc(const char *str2) const
	{
		return !_strnicmp(str2, str, sz) && (str2[sz] == 0);	//чтоб не просто проверка на подстроку была
	}

	//без учета регистра
	//сраниваются первый sz символов строк
	__forceinline bool equal_nc_sz(const char *str2) const
	{
		return !_strnicmp(str2, str, sz);
	}

//TODO: is_fisrt должна работать без вычисления длины
	//без учета регистра
	//!!!вычисляет длину, поэтому медленная
	__forceinline bool is_first_nc_slow(const char *sub_str) const
	{
		int len2 = strlen(sub_str);
		if(sz < len2)
			len2 = sz;
		return !_strnicmp(sub_str, str, len2);
	}

	//без учета регистра
	__forceinline bool is_first_nc(const char *sub_str, int sub_str_len) const
	{
		if(sz < sub_str_len)
			sub_str_len = sz;
		return !_strnicmp(sub_str, str, sub_str_len);
	}



	//Compare characters of two strings, using the current locale or a specified locale.
	//__forceinline int ncmp(const char *str2) const
	//{
	//	return strncmp(str2, str, sz);
	//}

	//Compare characters of two strings without regard to case.
	__forceinline int nicmp(const char *str2) const
	{
		return _strnicmp(str2, str, sz);
	}

	//устанавливает ноль в конец строки
	//return previous char
	__forceinline char push_last_zero()
	{
		//_ass(sz);
		char ret = str[sz];
		str[sz] = 0;
		return ret;
	}

	//устанавливает предыдущий символ
	__forceinline void pop_prev_char(char prev_c)
	{
		//_ass(sz);
		str[sz] = prev_c;
	}

	__forceinline zsubstr substr(int first, int count)
	{
		zsubstr ret;
		ret.str = this->str + first;
		ret.sz = count;
		if(first + count > sz)
			ret.sz = sz - first;
		return ret;
	}

	//обрубает с начала и конца пробельные символы " \t\n\r\xA0";
	void trim();

	std::string as_string_slow() const;

	__forceinline void remove_last_slash()
	{
		if(sz)
		{
			if(str[sz - 1] == '/')
				sz--;
		}
	}

};

