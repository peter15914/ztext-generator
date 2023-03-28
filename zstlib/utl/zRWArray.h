#pragma once

namespace zRWArray
{
	int _read_array(void *arr, const std::wstring &file_name, __int64 ind_first, __int64 ind_last);

	int _get_arr_size(const std::wstring &file_name, int type_size, __int64 ind_first, __int64 &ind_last);

	template<typename T>
	int read_array(std::vector<T> &vec, const std::wstring &file_name, __int64 ind_first, __int64 ind_last);

	//
	template<typename T>
	bool save_vec_to_file(const std::vector<T> &vec, std::wstring file_name);

	//читаем весь файл в память
	bool read_file_to_str(const wchar_t *file_name, char *_buf, int size);
	bool read_file_to_str(const wchar_t *file_name, std::string &buf);

	//пишем весь файл из памяти
	bool write_file_from_str(const wchar_t *file_name, const std::string &buf);
};
