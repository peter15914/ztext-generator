#pragma once

class zDbCashe;

//------------------------------------------------------------------------------------------
class row_cashe : public std::vector<std::string>
{
	zDbCashe *m_parent;
public:
	row_cashe();
	virtual ~row_cashe();

	void set_parent(zDbCashe *parent);

	std::vector<std::string>::const_reference operator[](const char *colName) const;
	std::vector<std::string>::reference operator[](const char *colName);
	std::vector<std::string>::const_reference operator[](std::string colName) const;
	std::vector<std::string>::reference operator[](std::string colName);

protected:
	std::vector<std::string>::const_reference operator[](size_t _Pos) const
	{
		return std::vector<std::string>::operator[] (_Pos);
	}

	std::vector<std::string>::reference operator[](size_t _Pos)
	{
		return std::vector<std::string>::operator[] (_Pos);
	}
};


//------------------------------------------------------------------------------------------
class zDbCashe : public boost::noncopyable
{
	std::vector<row_cashe> m_cashe;
	std::map<std::string, int> m_columns;

public:
	zDbCashe();
	virtual ~zDbCashe();

	//номер колонки
	size_t _get_col_num(const std::string &colName);

	inline bool empty() { return m_cashe.empty(); }
	inline size_t size() { return m_cashe.size(); }
	inline void resize(size_t _Newsize) { m_cashe.resize(_Newsize); }

	inline row_cashe &operator [](size_t index) { return m_cashe[index]; }

	void clear_columns();
	//добавляет колонку
	void add_column(const std::string &colName, int ind);
};
