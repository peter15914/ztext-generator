#pragma once

#include "../allocator.h"

class zDbCashe;

//------------------------------------------------------------------------------------------
struct zDataItem
{
	char *m_val;

	//zDataItem(const std::string &str) { this->assign(str); }
	__forceinline int AsInt() const { return m_val ? atoi(m_val) : 0; }
	__forceinline bool IsEmpty() const { return m_val == 0; }
	__forceinline std::string AsStr() const { return m_val; }

	__forceinline const char *get_val() const { return m_val; }
	__forceinline const char *get_val_safe() const { return m_val ? m_val : ""; }
};


//------------------------------------------------------------------------------------------
class zDataRow
{
	friend class zDbCashe;

	zDbCashe *m_parent;
	zDataItem *m_data;

private:
	zDataRow();
	~zDataRow();

public:
	void set_parent(zDbCashe *parent, zDataItem *data);

	const zDataItem& operator[](const char *colName) const;
	const zDataItem& operator[](std::string colName) const;
	zDataItem& operator[](const char *colName);
	zDataItem& operator[](std::string colName);

	zDataItem& get_item(int _Pos);

	void release_items();
};


//------------------------------------------------------------------------------------------
struct cmp_str
{
   bool operator()(char const *a, char const *b) const
   {
      return std::strcmp(a, b) < 0;
   }
};


//------------------------------------------------------------------------------------------
//<colname, мапа с мапами для быстрого поиска>
typedef std::map<const char*, int, cmp_str> DB_CASHE_SERCH_MAP;
//typedef std::map<std::string, DB_CASHE_SERCH_MAP*> DB_CASHE_SERCH_MAPS;

//------------------------------------------------------------------------------------------
struct DB_COLUMN
{
	std::string m_name;
	std::string m_real_name;	//из-за того, что при дублировании имен колонок добавляем суффикс
	bool is_indexed;
	bool is_case_sensitive;	//для индексированных колонок - учитывать ли case символов

	DB_CASHE_SERCH_MAP *m_index_map;

	DB_COLUMN();
	virtual ~DB_COLUMN();

	void add_val_to_index(int vec_ind, const char *value, StringPool &str_pool);
};


//------------------------------------------------------------------------------------------
class zDbCashe : public boost::noncopyable
{
	Allocator m_row_allocator;
	Allocator m_items_allocator;
	StringPool m_str_pool;

	std::vector<zDataRow*> m_rows;

	/// для колонок
	//std::vector<std::string> m_cols;
	//std::vector<std::string> m_col_real_name;	//из-за того, что при дублировании имен колонок добавляем суффикс

	std::vector<DB_COLUMN> m_cols_;

	//<name, ind>
	std::map<std::string, int> m_cache_cols;	//для быстроты поиска по имени

public:
	zDbCashe();
	virtual ~zDbCashe();

	/// колонки

	//количество колонок
	int get_columns_cnt() { return (int)m_cols_.size(); }
	std::string get_column_name(int ind);

	//номер колонки. INT_MAX - если ошибка
	size_t _get_col_num(const std::string &colName);

	void clear_columns();

	//
	inline bool empty() { return m_rows.empty(); }
	inline int size() { return (int)m_rows.size(); }
	inline void clear();

	inline zDataRow &operator [](size_t index) { return *m_rows[index]; }

	//создает пустой row в таблице и возвращает ссылку на на него
	int add_row();

	//добавляет колонку
	void add_column(int ind, const std::string &colName);

	//делает колонку индексированной (т.е. есть мапа для быстрого поиска)
	//теперь работает и для непустых таблиц
	void set_key_column(const char *colName, bool is_case_sensitive);

	//устанавливает только один раз при заполнении
	void set_value_on_fill(int vec_ind, int _Pos, const char *val);

	//return INT_MAX, если не нашла
	int find_in_col_(const char *colName, const char *_val);
};


//------------------------------------------------------------------------------------------
class zDbCasheSet : public boost::noncopyable
{
protected:
	std::vector<zDbCashe*> m_dbcashes;
	bool m_not_delete;	//не нужно уничтожать, т.к. создавалось не здесь

public:
	zDbCasheSet();
	virtual ~zDbCasheSet();

	void set_size(int sz);

	int size() { return (int)m_dbcashes.size(); }
	zDbCashe &get_db_cashe(int num) { return *m_dbcashes[num]; };

	bool empty() { return m_dbcashes.empty(); }

	void clear() { _clear_all(); }

	void add_foreign_db_cashe(zDbCashe *db_cashe);

protected:
	void _clear_all();
};


//------------------------------------------------------------------------------------------
class zDbCasheSetNamed : public zDbCasheSet
{
	//<tbl_name, ind>
	std::map<std::string, int> m_names_map;

public:
	zDbCasheSetNamed();
	virtual ~zDbCasheSetNamed();

	zDbCashe *add_one(const char *tbl_name);
	zDbCashe *get_db_cashe_by_name(const char *tbl_name);
};

