#include "stdafx.h"

#define ZRWARRAY_SILENCE


#include "zRWArray.h"

#include <zstlib/zutl.h>

#ifndef ZRWARRAY_SILENCE
	#include <zstlib/utl/iLog.h>
#endif


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//------------------------------------------------------------------------------------------
int zRWArray::_read_array(void *arr, const std::wstring &file_name, __int64 ind_first, __int64 ind_last)
{
	//открываем файл
	FILE *file1 = _wfopen(file_name.c_str(), L"rb");
	if(!file1)
		return (rel_assert(0), 0);

	//читаем из файла в массив
	_fseeki64(file1, ind_first, SEEK_SET);

	int size = (int)(ind_last-ind_first);
	int bb2 = fread(arr, 1, size, file1);
	rel_assert(bb2 == size);

	fclose(file1);
	return size;
}


//------------------------------------------------------------------------------------------
int zRWArray::_get_arr_size(const std::wstring &file_name, int type_size, __int64 ind_first, __int64 &ind_last)
{
	__int64 fsize1 = zutl::get_file_size(file_name);

	if(fsize1 % type_size != 0)
		return (rel_assert(0), 0);

	fsize1 /= type_size;

	//если надо, корректируем правую границу
	if(ind_last > fsize1)
	{
		#ifndef ZRWARRAY_SILENCE
			zdebug::log()->Log(zstr::fmt("ind_last corrected from %I64d to %I64d", ind_last, fsize1));
		#endif
		ind_last = fsize1;
	}

	//размер массива
	int size = (int)(ind_last - ind_first);
	if(size <= 0)
		return (rel_assert(0), 0);

	return size;
}


//------------------------------------------------------------------------------------------
template<typename T>
int zRWArray::read_array(std::vector<T> &vec, const std::wstring &file_name, __int64 ind_first, __int64 ind_last)
{

	int type_size = sizeof(T);

	int size = _get_arr_size(file_name, type_size, ind_first, ind_last);
	vec.resize(size);

	//читаем из файла
	_read_array(&vec[0], file_name, ind_first*type_size, ind_last*type_size);

	#ifndef ZRWARRAY_SILENCE
		zdebug::log()->Log("read_array end, " + zdebug::log()->GetLogTimeSpend() + "\n");
	#endif

	return size;
}


//------------------------------------------------------------------------------------------
template<typename T>
bool zRWArray::save_vec_to_file(const std::vector<T> &vec, std::wstring file_name)
{
	if(vec.empty())
		return (_ass(0), false);

	FILE *pFile = _wfopen(file_name.c_str(), L"wb");
	if(!pFile)
		return (rel_assert(0), false);

	int buf_written = fwrite(&vec[0], sizeof(T), vec.size(), pFile);
	rel_assert(buf_written == (int)vec.size());

	fclose(pFile);

	#ifndef ZRWARRAY_SILENCE
		zdebug::log()->Log("end save_vec_to_file");
	#endif

	return true;
}


//------------------------------------------------------------------------------------------
template int zRWArray::read_array(std::vector<__int64> &vec, const std::wstring &file_name, __int64 ind_first, __int64 ind_last);
template int zRWArray::read_array(std::vector<unsigned int> &vec, const std::wstring &file_name, __int64 ind_first, __int64 ind_last);
template int zRWArray::read_array(std::vector<int> &vec, const std::wstring &file_name, __int64 ind_first, __int64 ind_last);
template int zRWArray::read_array(std::vector<unsigned char> &vec, const std::wstring &file_name, __int64 ind_first, __int64 ind_last);
template bool zRWArray::save_vec_to_file(const std::vector<__int64> &vec, std::wstring file_name);
template bool zRWArray::save_vec_to_file(const std::vector<unsigned int> &vec, std::wstring file_name);
template bool zRWArray::save_vec_to_file(const std::vector<int> &vec, std::wstring file_name);
template bool zRWArray::save_vec_to_file(const std::vector<unsigned short> &vec, std::wstring file_name);
template bool zRWArray::save_vec_to_file(const std::vector<unsigned char> &vec, std::wstring file_name);


//------------------------------------------------------------------------------------------
//читаем весь файл в память
bool zRWArray::read_file_to_str(const wchar_t *file_name, char *_buf, int size)
{
	//открываем файл
	FILE *file1 = _wfopen(file_name, L"rb");
	if(!file1)
		return (rel_assert(0), false);

	//читаем весь файл в память
	int bb2 = (int)fread(_buf, 1, size, file1);
	rel_assert(bb2 == size);

	_buf[size] = 0;

	fclose(file1);
	return true;
}


//------------------------------------------------------------------------------------------
bool zRWArray::read_file_to_str(const wchar_t *file_name, std::string &buf)
{
	//размер файла
	int size = (int)zutl::get_file_size(file_name);
	if(!size)
		return (rel_assert(0), false);

	//выделяем память
	buf.resize(size);

	return read_file_to_str(file_name, &buf[0], (int)buf.size());
}


//------------------------------------------------------------------------------------------
//пишем весь файл из памяти
bool zRWArray::write_file_from_str(const wchar_t *file_name, const std::string &buf)
{
	//открываем файл
	FILE *file1 = _wfopen(file_name, L"wb");
	if(!file1)
		return (rel_assert(0), false);

	//пишем
	int size = (int)buf.size();
	int bb2 = (int)fwrite(&buf[0], 1, size, file1);
	rel_assert(bb2 == size);

	fclose(file1);
	return true;
}