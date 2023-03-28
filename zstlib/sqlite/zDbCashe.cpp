#include "stdafx.h"

#include "zDbCashe.h"

#include <zstlib/textutl.h>


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace std;

//------------------------------------------------------------------------------------------
zDataRow::zDataRow()
{
	m_parent = 0;
	m_data = 0;
}


//------------------------------------------------------------------------------------------
zDataRow::~zDataRow()
{
}


//------------------------------------------------------------------------------------------
void zDataRow::set_parent(zDbCashe *parent, zDataItem *data)
{
	m_parent = parent;
	m_data = data;
}


//------------------------------------------------------------------------------------------
const zDataItem& zDataRow::operator[](const char *colName) const
{
	size_t _Pos = m_parent->_get_col_num(colName);
	_ass(_Pos != INT_MAX);
	return m_data[_Pos];
}


//------------------------------------------------------------------------------------------
zDataItem& zDataRow::operator[](const char *colName)
{
	size_t _Pos = m_parent->_get_col_num(colName);
	_ass(_Pos != INT_MAX);
	return m_data[_Pos];
}


//------------------------------------------------------------------------------------------
const zDataItem& zDataRow::operator[](std::string colName) const
{
	return this->operator[] (colName.c_str());
}


//------------------------------------------------------------------------------------------
zDataItem& zDataRow::operator[](std::string colName)
{
	return this->operator[] (colName.c_str());
}


//------------------------------------------------------------------------------------------
zDataItem& zDataRow::get_item(int _Pos)
{
	return m_data[_Pos];
}


//------------------------------------------------------------------------------------------
void zDataRow::release_items()
{
}


//------------------------------------------------------------------------------------------
DB_COLUMN::DB_COLUMN() :
	is_indexed(false),
	m_index_map(0),
	is_case_sensitive(true)
{
	
}


//------------------------------------------------------------------------------------------
DB_COLUMN::~DB_COLUMN()
{
	if(m_index_map)
		delete m_index_map;
	m_index_map = 0;
}


//------------------------------------------------------------------------------------------
zDbCashe::zDbCashe() :
	m_row_allocator(sizeof(zDataRow), 1024 * 16)
{
	dbg_assert(sizeof(zDataItem) == sizeof(char*));
	dbg_assert(sizeof(zDataRow) == 8);

	m_rows.reserve(1024);
}


//------------------------------------------------------------------------------------------
zDbCashe::~zDbCashe()
{
	clear();
}


//------------------------------------------------------------------------------------------
//номер колонки
size_t zDbCashe::_get_col_num(const std::string &colName)
{
	size_t ret = INT_MAX;

	map<string, int>::iterator it = m_cache_cols.find(colName);
	if(it != m_cache_cols.end())
		ret = it->second;

	return ret;
}


//------------------------------------------------------------------------------------------
void zDbCashe::clear_columns()
{
	m_cols_.clear();
	m_cache_cols.clear();
}


//------------------------------------------------------------------------------------------
//добавляет колонку
void zDbCashe::add_column(int ind, const std::string &colName)
{
	bool need_add = true;
	string new_col_name = colName;

	map<string, int>::iterator it = m_cache_cols.find(colName);
	if(it == m_cache_cols.end())
	{
		//не было колонки с таким именем, всё нормально
		need_add = true;
	}
	else
	{
		if(it->second == ind)
		{
			//такая колонка есть, добавлять не нужно
			need_add = false;
		}
		else
		{
			//если уже есть такая колонка, но индекс другой - добавляем "_1", потом "_2", и т.д. до "_9"

			int suffix = 0;
			while(++suffix < 10)
			{
				string new_col_name2 = colName + (boost::format("_%d") % suffix).str();

				it = m_cache_cols.find(new_col_name2);

				if(it == m_cache_cols.end())
				{
					new_col_name = new_col_name2;
					need_add = true;
					break;
				}
				else
				if(it->second == ind)
				{
					need_add = false;
					break;
				}
			}
			_ass(suffix < 10);
		}
	}

	//добавляем, если надо
	if(need_add)
	{
		DB_COLUMN buf;

		buf.m_name = new_col_name;
		buf.m_real_name = colName;
		m_cols_.push_back(buf);
		//m_col_real_name.push_back();

		m_cache_cols[new_col_name] = ind;
	}
}


//------------------------------------------------------------------------------------------
//делает колонку индексированной (т.е. есть мапа для быстрого поиска)
void zDbCashe::set_key_column(const char *colName, bool is_case_sensitive)
{
	size_t ind = _get_col_num(colName);
	if(ind == INT_MAX)
		return (_ass(0));

	if(m_cols_[ind].is_indexed || m_cols_[ind].m_index_map)
		return (_ass(0));	//уже вызывали

	DB_COLUMN &col = m_cols_[ind];
	col.is_indexed = true;
	col.m_index_map = new DB_CASHE_SERCH_MAP();
	col.is_case_sensitive = is_case_sensitive;

	//если таблица непустая, то нужно прямо пробежать и заполнить мапу
	if(!m_rows.empty())
	{
		for(int i = 0; i < this->size(); i++)
		{
			zDataRow &row = *m_rows[i];
			col.add_val_to_index(i, row.get_item(ind).get_val_safe(), m_str_pool);
		}
	}
}


//------------------------------------------------------------------------------------------
//создает пустой row в таблице и возвращает ссылку на на него
int zDbCashe::add_row()
{
	int cols = get_columns_cnt();
	int mem_size = cols * sizeof(zDataItem);
	if(empty())
	{
		//самый первый раз задаем параметры для аллокатора
		m_items_allocator.SetParams(mem_size, 1024 * 16);
	}

	//создаем новый row
	size_t sz = size();
	m_rows.resize(sz + 1);

	zDataRow *new_row = (zDataRow *)m_row_allocator.GetMemory();
	m_rows[sz] = new_row;

	//
	zDataItem *data = (zDataItem *)m_items_allocator.GetMemory();
	memset(data, 0, mem_size);

	new_row->set_parent(this, data);

	return sz;
}


//------------------------------------------------------------------------------------------
std::string zDbCashe::get_column_name(int ind)
{
	if(ind < 0 || ind >= (int)m_cols_.size())
		return (rel_assert(0), string());

	return m_cols_[ind].m_name;
}


//------------------------------------------------------------------------------------------
void zDbCashe::clear()
{
	for(int i = 0; i < (int)m_rows.size(); i++)
		m_rows[i]->release_items();

	m_rows.clear();
	m_row_allocator.FreeAll();

	m_items_allocator.FreeAll();

	m_str_pool.FreeAll();

	for(int i = 0; i < (int)m_cols_.size(); i++)
	{
		if(m_cols_[i].is_indexed && m_cols_[i].m_index_map)
			m_cols_[i].m_index_map->clear();
	}
}


//------------------------------------------------------------------------------------------
//устанавливает только один раз при заполнении
void zDbCashe::set_value_on_fill(int vec_ind, int _Pos, const char *val)
{
	zDataRow &vec = *m_rows[vec_ind];

	zDataItem *data = vec.m_data;
	if(data[_Pos].m_val)
		return (_ass(0));

	if(!val)
		return;

	int len = (int)strlen(val);
	char *new_str = m_str_pool.AllocString_(val, len);
	data[_Pos].m_val = new_str;

	//для индексированных колонок добавляем в мапу
	DB_COLUMN &cur_col = m_cols_[_Pos];
	if(cur_col.is_indexed)
		cur_col.add_val_to_index(vec_ind, new_str, m_str_pool);
}


//------------------------------------------------------------------------------------------
void DB_COLUMN::add_val_to_index(int vec_ind, const char *value, StringPool &str_pool)
{
	if(is_case_sensitive)
		(*m_index_map)[value] = vec_ind;
	else
	{
		string up = value;
		textutl::to_bigger_eng(up, 0);
		textutl::to_bigger_rus(up, 0);

		char *upcase_val = str_pool.AllocString_(&up[0], (int)up.size());
		(*m_index_map)[upcase_val] = vec_ind;
	}
}


//------------------------------------------------------------------------------------------
//return INT_MAX, если не нашла
int zDbCashe::find_in_col_(const char *colName, const char *_val)
{
	size_t col_ind = _get_col_num(colName);
	if(col_ind == INT_MAX)
		return (_ass(0), INT_MAX);

	DB_COLUMN &cur_col = m_cols_[col_ind];
	if(!cur_col.is_indexed || !cur_col.m_index_map)
		return (_ass(0), INT_MAX);	//колонка не индексированная

	DB_CASHE_SERCH_MAP &ind_map = *cur_col.m_index_map;

	DB_CASHE_SERCH_MAP::iterator it;
	if(cur_col.is_case_sensitive)
		it = ind_map.find(_val);
	else
	{
		string up(_val);
		textutl::to_bigger_eng(up);
		textutl::to_bigger_rus(up);
		it = ind_map.find(up.c_str());
	}

	if(it == ind_map.end())
		return INT_MAX;
	else
		return it->second;
}


//------------------------------------------------------------------------------------------
zDbCasheSet::zDbCasheSet() :
	m_not_delete(false)
{
}


//------------------------------------------------------------------------------------------
zDbCasheSet::~zDbCasheSet()
{
	_clear_all();
}


//------------------------------------------------------------------------------------------
void zDbCasheSet::_clear_all()
{
	if(!m_not_delete)
	{
		for(int i = 0; i < (int)m_dbcashes.size(); i++)
			delete m_dbcashes[i];
	}
	m_dbcashes.clear();
}


//------------------------------------------------------------------------------------------
void zDbCasheSet::set_size(int sz)
{
	if(!m_dbcashes.empty())
		return (_ass(0));		//используй только свежий или вызови clean() перед этим

	for(int i = 0; i < sz; i++)
		m_dbcashes.push_back(new zDbCashe());
}


//------------------------------------------------------------------------------------------
void zDbCasheSet::add_foreign_db_cashe(zDbCashe *db_cashe)
{
	//проверям, что нет ошибки
	if(!m_dbcashes.empty())
	{
		if(!m_not_delete)
			return (_ass(0));
	}

	m_not_delete = true;
	m_dbcashes.push_back(db_cashe);
}


//------------------------------------------------------------------------------------------
zDbCasheSetNamed::zDbCasheSetNamed()
{
}


//------------------------------------------------------------------------------------------
zDbCasheSetNamed::~zDbCasheSetNamed()
{
}


//------------------------------------------------------------------------------------------
zDbCashe *zDbCasheSetNamed::add_one(const char *tbl_name)
{
	zDbCashe *ret = new zDbCashe();
	m_dbcashes.push_back(ret);

	m_names_map[tbl_name] = (int)m_dbcashes.size() - 1;

	return ret;
}


//------------------------------------------------------------------------------------------
zDbCashe *zDbCasheSetNamed::get_db_cashe_by_name(const char *tbl_name)
{
	std::map<std::string, int>::iterator it = m_names_map.find(tbl_name);
	if(it == m_names_map.end())
		return 0;

	int ind = it->second;
	if(ind < 0 || ind >= (int)m_dbcashes.size())
		return (_ass(0), 0);

	return m_dbcashes[ind];
}

