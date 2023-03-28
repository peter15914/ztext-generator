#pragma once

//------------------------------------------------------------------------------
namespace zsqlutils
{
	//конвертнуть строку для записи в sql-дамп
	//bad_quote_char - это обычно '\'' или '"'
	std::string _convert_str_for_sql_statement(const std::string &s, char bad_quote_char);

	//для sqlite
	inline std::string convert_str_for_sqlite(const std::string &s)
	{
		return _convert_str_for_sql_statement(s, '"');
	}

	//для MySQL
	inline std::string convert_str_for_mysql(const std::string &s)
	{
		return _convert_str_for_sql_statement(s, '\'');
	}

};
