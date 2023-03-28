#include "stdafx.h"

#include "zDbCashe.h"

using namespace std;

//------------------------------------------------------------------------------------------
row_cashe::row_cashe()
{
	m_parent = 0;
}


//------------------------------------------------------------------------------------------
row_cashe::~row_cashe()
{
}


//------------------------------------------------------------------------------------------
void row_cashe::set_parent(zDbCashe *parent)
{
	m_parent = parent;
}


//------------------------------------------------------------------------------------------
std::vector<std::string>::const_reference row_cashe::operator[](const char *colName) const
{
	size_t _Pos = m_parent->_get_col_num(colName);
	_ass(_Pos != INT_MAX);
	return std::vector<std::string>::operator[] (_Pos);
}


//------------------------------------------------------------------------------------------
std::vector<std::string>::reference row_cashe::operator[](const char *colName)
{
	size_t _Pos = m_parent->_get_col_num(colName);
	_ass(_Pos != INT_MAX);
	return std::vector<std::string>::operator[] (_Pos);
}


//------------------------------------------------------------------------------------------
std::vector<std::string>::const_reference row_cashe::operator[](std::string colName) const
{
	return this->operator[] (colName.c_str());
}


//------------------------------------------------------------------------------------------
std::vector<std::string>::reference row_cashe::operator[](std::string colName)
{
	return this->operator[] (colName.c_str());
}


//------------------------------------------------------------------------------------------
zDbCashe::zDbCashe()
{
}


//------------------------------------------------------------------------------------------
zDbCashe::~zDbCashe()
{
}


//------------------------------------------------------------------------------------------
//номер колонки
size_t zDbCashe::_get_col_num(const std::string &colName)
{
	size_t ret = INT_MAX;

	map<string, int>::iterator it = m_columns.find(colName);
	if(it != m_columns.end())
		ret = it->second;
	

	return ret;
}


//------------------------------------------------------------------------------------------
void zDbCashe::clear_columns()
{
	m_columns.clear();
}


//------------------------------------------------------------------------------------------
//добавляет колонку
void zDbCashe::add_column(const std::string &colName, int ind)
{
	map<string, int>::iterator it = m_columns.find(colName);
	if(it != m_columns.end())
	{
		_ass(it->second == ind);
	}
	else
		m_columns[colName] = ind;
}

