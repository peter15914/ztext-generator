#pragma once

#include <zstlib\ztypes.h>

namespace textutl
{
	//приводит к нижнему/верхнему регистру все буквы строки, начиная с символа под номером beg
	void to_lower_eng(std::string &str, size_t beg);
	void to_lower_rus(std::string &str, size_t beg);
	void to_bigger_eng(std::string &str, size_t beg);
	void to_bigger_rus(std::string &str, size_t beg);

	__forceinline void to_lower_eng(std::string &str)
	{
		to_lower_eng(str, 0);
	}
	__forceinline void to_lower_rus(std::string &str)
	{
		to_lower_rus(str, 0);
	}
	__forceinline void to_bigger_eng(std::string &str)
	{
		to_bigger_eng(str, 0);
	}
	__forceinline void to_bigger_rus(std::string &str)
	{
		to_bigger_rus(str, 0);
	}

	void to_translit(const std::string &str, std::string &ret);
	std::string to_translit_copy(const std::string &str);

	void get_translit(char c, char &ret_c1, char &ret_c2);

	char get_lower_eng(char c);
	char get_lower_rus(char c);
	char get_bigger_eng(char c);
	char get_bigger_rus(char c);

	bool is_letter_rus(char c);
	bool is_letter_eng(char c);

	__forceinline bool is_numeric(char c);

	std::string get_rand_str(int len);

	bool has_rus_letters(const std::string &str);

	ZBOOL equal_nc(const char *str1, const char *str2);
};


//------------------------------------------------------------------------------------------
bool textutl::is_numeric(char c)
{
	return (c >= '0') && (c <= '9');
}
